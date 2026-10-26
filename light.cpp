#include "light.h"
#include <unordered_set>

#include "dirent.h"
#include "formfilter.h"
#include "parsetable.h"
#include "utility.h"

#include <cmath>
#include <fstream>
#include <limits>
#include <sys/stat.h>

namespace LIGHT {
namespace {
#define LIGHT_F(member)                                                        \
  static_cast<std::uint16_t>(offsetof(patch_instruction, member))

constexpr sp::KeySpec kLightKeys[] = {
    {"filterByLights", sp::FieldKind::List, LIGHT_F(object)},
    {"filterByLightsExcluded", sp::FieldKind::List, LIGHT_F(objectExcluded)},
    {"filterByModNames", sp::FieldKind::ListAssign, LIGHT_F(modNames)},
    {"filterByEditorIdContains", sp::FieldKind::ListAssign,
     LIGHT_F(filterByEditorIdContains)},
    {"filterByEditorIdContainsOr", sp::FieldKind::ListAssign,
     LIGHT_F(filterByEditorIdContainsOr)},
    {"filterByEditorIdContainsExcluded", sp::FieldKind::ListAssign,
     LIGHT_F(filterByEditorIdContainsExcluded)},
    {"time", sp::FieldKind::Value, LIGHT_F(time)},
    {"radius", sp::FieldKind::Value, LIGHT_F(radius)},
    {"color", sp::FieldKind::ListAssign, LIGHT_F(color)},
    {"fade", sp::FieldKind::Value, LIGHT_F(fade)},
    {"falloffExponent", sp::FieldKind::Value, LIGHT_F(falloffExponent)},
    {"fov", sp::FieldKind::Value, LIGHT_F(fov)},
    {"nearDistance", sp::FieldKind::Value, LIGHT_F(nearDistance)},
    {"flickerPeriod", sp::FieldKind::Value, LIGHT_F(flickerPeriod)},
    {"flickerIntensityAmplitude", sp::FieldKind::Value,
     LIGHT_F(flickerIntensityAmplitude)},
    {"flickerMovementAmplitude", sp::FieldKind::Value,
     LIGHT_F(flickerMovementAmplitude)},
    {"setFlags", sp::FieldKind::ListAssign, LIGHT_F(setFlags)},
    {"removeFlags", sp::FieldKind::ListAssign, LIGHT_F(removeFlags)},
};

#undef LIGHT_F

const sp::KeyTable &lightKeyTable() {
  static const sp::KeyTable table(kLightKeys, std::size(kLightKeys));
  return table;
}

struct FlagRow {
  std::string_view name;
  RE::TES_LIGHT_FLAGS flag;
};

constexpr FlagRow kLightFlags[] = {
    {"dynamic", RE::TES_LIGHT_FLAGS::kDynamic},
    {"cancarry", RE::TES_LIGHT_FLAGS::kCanCarry},
    {"negative", RE::TES_LIGHT_FLAGS::kNegative},
    {"flicker", RE::TES_LIGHT_FLAGS::kFlicker},
    {"deepcopy", RE::TES_LIGHT_FLAGS::kDeepCopy},
    {"offbydefault", RE::TES_LIGHT_FLAGS::kOffByDefault},
    {"flickerslow", RE::TES_LIGHT_FLAGS::kFlickerSlow},
    {"pulse", RE::TES_LIGHT_FLAGS::kPulse},
    {"pulseslow", RE::TES_LIGHT_FLAGS::kPulseSlow},
    {"spotlight", RE::TES_LIGHT_FLAGS::kSpotlight},
    {"spotshadow", RE::TES_LIGHT_FLAGS::kSpotShadow},
    {"hemishadow", RE::TES_LIGHT_FLAGS::kHemiShadow},
    {"omnishadow", RE::TES_LIGHT_FLAGS::kOmniShadow},
    {"portalstrict", RE::TES_LIGHT_FLAGS::kPortalStrict},
};

const FlagRow *findFlag(const std::string &raw) {
  const auto lowered = toLowerCase(raw);
  for (const auto &row : kLightFlags) {
    if (row.name == lowered) {
      return &row;
    }
  }
  return nullptr;
}

template <class T> bool parseInteger(const std::string &text, T &result) {
  try {
    std::size_t parsed = 0;
    if constexpr (std::is_signed_v<T>) {
      const auto value = std::stoll(text, &parsed, 10);
      if (parsed != text.size() || value < std::numeric_limits<T>::min() ||
          value > std::numeric_limits<T>::max()) {
        return false;
      }
      result = static_cast<T>(value);
    } else {
      if (!text.empty() && text.front() == '-') {
        return false;
      }
      const auto value = std::stoull(text, &parsed, 10);
      if (parsed != text.size() || value > std::numeric_limits<T>::max()) {
        return false;
      }
      result = static_cast<T>(value);
    }
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool parseFloat(const std::string &text, float &result) {
  try {
    std::size_t parsed = 0;
    const float value = std::stof(text, &parsed);
    if (parsed != text.size() || !std::isfinite(value)) {
      return false;
    }
    result = value;
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

template <class T>
void applyInteger(const std::string &text, std::string_view field,
                  RE::TESObjectLIGH *light, T &target) {
  if (text.empty() || text == "none") {
    return;
  }
  T value{};
  if (!parseInteger(text, value)) {
    logger::warn("light formid: {:08X} invalid {} '{}'", light->formID, field,
                 text);
    return;
  }
  target = value;
  logger::debug("light formid: {:08X} changed {} to {}", light->formID, field,
                target);
}

void applyFloat(const std::string &text, std::string_view field,
                RE::TESObjectLIGH *light, float &target, float scale = 1.0F) {
  if (text.empty() || text == "none") {
    return;
  }
  float value{};
  if (!parseFloat(text, value) || !std::isfinite(value * scale)) {
    logger::warn("light formid: {:08X} invalid {} '{}'", light->formID, field,
                 text);
    return;
  }
  target = value * scale;
  logger::debug("light formid: {:08X} changed {} to {}", light->formID, field,
                value);
}
}

patch_instruction create_patch_instructions(const std::string &line) {
  patch_instruction result;
  lightKeyTable().parse(line, &result);
  return result;
}

void process_patch_instructions(const std::list<patch_instruction> &tokens) {
  logger::debug("processing light patch instructions");
  const auto dataHandler = RE::TESDataHandler::GetSingleton();
  auto &lights = dataHandler->GetFormArray<RE::TESObjectLIGH>();

  for (const auto &line : tokens) {
    std::unordered_set<RE::FormID> patched;
    for (const auto &identifier : line.object) {
      if (auto *form = GetFormFromIdentifier(identifier);
          form && form->formType == RE::FormType::Light) {
        if (patched.insert((static_cast<RE::TESObjectLIGH *>(form))->formID).second) {
          patch(line, static_cast<RE::TESObjectLIGH *>(form));
        }
      }
    }

    if (!line.object.empty() && line.filterByEditorIdContains.empty() &&
        line.filterByEditorIdContainsOr.empty()) {
      continue;
    }

    const formfilter::CommonFilterView view{
        line.modNames,
        formfilter::noStrings(),
        formfilter::noStrings(),
        formfilter::noStrings(),
        formfilter::noStrings(),
        formfilter::noStrings(),
        line.filterByEditorIdContains,
        line.filterByEditorIdContainsOr,
        line.filterByEditorIdContainsExcluded,
        line.objectExcluded};

    for (auto *light : lights) {
      if (!light || light->IsDeleted()) {
        continue;
      }
      if (!line.modNames.empty() && !formfilter::matchesModName(view, light)) {
        continue;
      }

      bool found = line.object.empty() &&
                   line.filterByEditorIdContains.empty() &&
                   line.filterByEditorIdContainsOr.empty();
      const bool contains = line.filterByEditorIdContains.empty() ||
                            formfilter::matchesEditorIdAnd(view, light);
      const bool containsOr = line.filterByEditorIdContainsOr.empty() ||
                              formfilter::matchesEditorIdOr(view, light);
      if ((!line.filterByEditorIdContains.empty() ||
           !line.filterByEditorIdContainsOr.empty()) &&
          contains && containsOr) {
        found = true;
      }
      if (found && !line.objectExcluded.empty() &&
          formfilter::isObjectExcluded(view, light, RE::FormType::Light)) {
        found = false;
      }
      if (found && !line.filterByEditorIdContainsExcluded.empty() &&
          formfilter::hasExcludedEditorId(view, light)) {
        found = false;
      }
      if (found) {
        if (!patched.contains(light->formID)) {
          patch(line, light);
        }
      }
    }
  }
}

void patch(const patch_instruction &line, RE::TESObjectLIGH *light) {
  applyInteger(line.time, "time", light, light->data.time);
  applyInteger(line.radius, "radius", light, light->data.radius);
  applyFloat(line.fade, "fade", light, light->fade);
  applyFloat(line.falloffExponent, "falloffExponent", light,
             light->data.fallofExponent);
  applyFloat(line.fov, "fov", light, light->data.fov);
  applyFloat(line.nearDistance, "nearDistance", light,
             light->data.nearDistance);
  if (!line.flickerPeriod.empty() && line.flickerPeriod != "none") {
    float period{};
    if (!parseFloat(line.flickerPeriod, period) || !std::isfinite(period) ||
        period < 0.0F) {
      logger::warn("light formid: {:08X} invalid flickerPeriod '{}'",
                   light->formID, line.flickerPeriod);
    } else if (period == 0.0F) {
      light->data.flickerPeriodRecip = 0.0F;
      logger::debug("light formid: {:08X} changed flickerPeriod to 0",
                    light->formID);
    } else {
      light->data.flickerPeriodRecip = 1.0F / (std::max)(period, 1.0F);
      logger::debug("light formid: {:08X} changed flickerPeriod to {}",
                    light->formID, period);
    }
  }
  applyFloat(line.flickerIntensityAmplitude, "flickerIntensityAmplitude", light,
             light->data.flickerIntensityAmplitude);
  applyFloat(line.flickerMovementAmplitude, "flickerMovementAmplitude", light,
             light->data.flickerMovementAmplitude);

  if (!line.color.empty()) {
    if (line.color.size() != 3) {
      logger::warn("light formid: {:08X} color requires red,green,blue",
                   light->formID);
    } else {
      std::uint8_t red{};
      std::uint8_t green{};
      std::uint8_t blue{};
      if (parseInteger(line.color[0], red) &&
          parseInteger(line.color[1], green) &&
          parseInteger(line.color[2], blue)) {
        light->data.color.red = red;
        light->data.color.green = green;
        light->data.color.blue = blue;
        logger::debug("light formid: {:08X} changed color to {},{},{}",
                      light->formID, red, green, blue);
      } else {
        logger::warn(
            "light formid: {:08X} invalid color; RGB values must be 0-255",
            light->formID);
      }
    }
  }

  for (const auto &raw : line.setFlags) {
    if (const auto *row = findFlag(raw)) {
      light->data.flags.set(row->flag);
      logger::debug("light formid: {:08X} flag enabled {}", light->formID,
                    row->name);
    } else {
      logger::warn("light formid: {:08X} unknown flag '{}' in setFlags",
                   light->formID, raw);
    }
  }
  for (const auto &raw : line.removeFlags) {
    if (const auto *row = findFlag(raw)) {
      light->data.flags.reset(row->flag);
      logger::debug("light formid: {:08X} flag disabled {}", light->formID,
                    row->name);
    } else {
      logger::warn("light formid: {:08X} unknown flag '{}' in removeFlags",
                   light->formID, raw);
    }
  }
}

void *readConfig(const std::string &folder) {
  constexpr char skipChar = ';';
  const std::string extension = ".ini";
  std::list<std::string> directories{folder};

  while (!directories.empty()) {
    const std::string currentFolder = directories.front();
    directories.pop_front();
    DIR *dir = opendir(currentFolder.c_str());
    if (!dir) {
      logger::info("Couldn't open directory {}.", currentFolder);
      continue;
    }

    while (const auto *ent = readdir(dir)) {
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
        continue;
      }
      const std::string fullPath = currentFolder + "\\" + ent->d_name;
      struct _stat statBuffer {};
      if (_stat(fullPath.c_str(), &statBuffer) == 0 &&
          (_S_IFDIR & statBuffer.st_mode)) {
        directories.push_back(fullPath);
        continue;
      }

      std::string fileName = ent->d_name;
      const auto extensionPosition = fileName.find(extension);
      if (extensionPosition == std::string::npos) {
        continue;
      }
      fileName = fileName.substr(0, extensionPosition);
      const std::string lowerName = toLowerCase(fileName);
      if ((lowerName.find(".esp") != std::string::npos ||
           lowerName.find(".esl") != std::string::npos ||
           lowerName.find(".esm") != std::string::npos) &&
          !IsPluginInstalled(fileName.c_str())) {
        logger::info("{} not found or is not a valid plugin file, skipping "
                     "config file {}.",
                     fileName, fullPath);
        continue;
      }

      logger::info("Processing config file {}... ", fullPath);
      std::ifstream infile(fullPath);
      std::list<patch_instruction> tokens;
      std::string configLine;
      while (std::getline(infile, configLine)) {
        configLine = StripLineComment(configLine);
        if (!configLine.empty() && configLine[0] != skipChar) {
          tokens.push_back(create_patch_instructions(configLine));
        }
      }
      process_patch_instructions(tokens);
    }
    closedir(dir);
  }
  return nullptr;
}
}
