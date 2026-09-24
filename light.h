#ifndef LIGHT_H
#define LIGHT_H

#include "PCH.h"

#include <list>
#include <string>
#include <vector>

namespace LIGHT {
struct patch_instruction {
  std::vector<std::string> object;
  std::vector<std::string> objectExcluded;
  std::vector<std::string> modNames;
  std::vector<std::string> filterByEditorIdContains;
  std::vector<std::string> filterByEditorIdContainsOr;
  std::vector<std::string> filterByEditorIdContainsExcluded;

  std::string time;
  std::string radius;
  std::vector<std::string> color;
  std::string fade;
  std::string falloffExponent;
  std::string fov;
  std::string nearDistance;
  std::string flickerPeriod;
  std::string flickerIntensityAmplitude;
  std::string flickerMovementAmplitude;
  std::vector<std::string> setFlags;
  std::vector<std::string> removeFlags;
};

patch_instruction create_patch_instructions(const std::string &line);
void process_patch_instructions(const std::list<patch_instruction> &tokens);
void *readConfig(const std::string &folder);
void patch(const patch_instruction &line, RE::TESObjectLIGH *curobj);
}

#endif
