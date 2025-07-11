#ifndef FNSOLVER_DATA_PRECIOUS_RESOURCE_H
#define FNSOLVER_DATA_PRECIOUS_RESOURCE_H

#include <string>
#include <unordered_map>

namespace precious_resource {

enum class Type {
  arc_sand_ore,
  aurorite,
  white_cometite,
  enduron_lead,
  everfreeze_ore,
  foucaultium,
  lionbone_bort,
  infernium,
  boiled_egg_ore,
  marine_rutile,
  dawnstone,
  cimmerian_cinnabar,
  ouroboros_crystal,
  parhelion_platinum,
  bonjelium
};

constexpr size_t count = 15;
extern const std::unordered_map<std::string, Type> type_for_str;
extern const std::unordered_map<Type, std::string> name_for_type;

}

#endif // FNSOLVER_DATA_PRECIOUS_RESOURCE_H

