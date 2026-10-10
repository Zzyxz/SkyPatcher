#include <RE/E/ExtraCellAcousticSpace.h>
#include <RE/E/ExtraCellImageSpace.h>
#include <RE/E/ExtraCellMusicType.h>
#include <RE/E/ExtraCellSkyRegion.h>
#include <RE/E/ExtraLocation.h>

namespace RE
{
	ExtraCellAcousticSpace::~ExtraCellAcousticSpace() = default;
	ExtraDataType ExtraCellAcousticSpace::GetType() const { return EXTRADATATYPE; }
	bool ExtraCellAcousticSpace::IsNotEqual(const BSExtraData* rhs) const
	{
		auto* other = rhs ? static_cast<const ExtraCellAcousticSpace*>(rhs) : nullptr;
		return !other || space != other->space;
	}

	ExtraCellImageSpace::~ExtraCellImageSpace() = default;
	ExtraDataType ExtraCellImageSpace::GetType() const { return EXTRADATATYPE; }
	bool ExtraCellImageSpace::IsNotEqual(const BSExtraData* rhs) const
	{
		auto* other = rhs ? static_cast<const ExtraCellImageSpace*>(rhs) : nullptr;
		return !other || imageSpace != other->imageSpace;
	}

	ExtraCellMusicType::~ExtraCellMusicType() = default;
	ExtraDataType ExtraCellMusicType::GetType() const { return EXTRADATATYPE; }
	bool ExtraCellMusicType::IsNotEqual(const BSExtraData* rhs) const
	{
		auto* other = rhs ? static_cast<const ExtraCellMusicType*>(rhs) : nullptr;
		return !other || type != other->type;
	}

	ExtraCellSkyRegion::~ExtraCellSkyRegion() = default;
	ExtraDataType ExtraCellSkyRegion::GetType() const { return EXTRADATATYPE; }
	bool ExtraCellSkyRegion::IsNotEqual(const BSExtraData* rhs) const
	{
		auto* other = rhs ? static_cast<const ExtraCellSkyRegion*>(rhs) : nullptr;
		return !other || skyRegion != other->skyRegion;
	}

	ExtraLocation::~ExtraLocation() = default;
	ExtraDataType ExtraLocation::GetType() const { return EXTRADATATYPE; }
	bool ExtraLocation::IsNotEqual(const BSExtraData* rhs) const
	{
		auto* other = rhs ? static_cast<const ExtraLocation*>(rhs) : nullptr;
		return !other || location != other->location;
	}
}
