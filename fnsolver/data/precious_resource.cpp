#include <fnsolver/data/precious_resource.h>

#include <string>
#include <unordered_map>

const std::unordered_map<std::string, precious_resource::Type> precious_resource::type_for_str = {
  {"arc_sand_ore", Type::arc_sand_ore},
  {"aurorite", Type::aurorite},
  {"white_cometite", Type::white_cometite},
  {"enduron_lead", Type::enduron_lead},
  {"everfreeze_ore", Type::everfreeze_ore},
  {"foucaultium", Type::foucaultium},
  {"lionbone_bort", Type::lionbone_bort},
  {"infernium", Type::infernium},
  {"boiled_egg_ore", Type::boiled_egg_ore},
  {"marine_rutile", Type::marine_rutile},
  {"dawnstone", Type::dawnstone},
  {"cimmerian_cinnabar", Type::cimmerian_cinnabar},
  {"ouroboros_crystal", Type::ouroboros_crystal},
  {"parhelion_platinum", Type::parhelion_platinum},
  {"bonjelium", Type::bonjelium}
};

const std::unordered_map<precious_resource::Type, std::string> precious_resource::name_for_type = {
  {Type::arc_sand_ore, "Arc Sand Ore"},
  {Type::aurorite, "Aurorite"},
  {Type::white_cometite, "White Cometite"},
  {Type::enduron_lead, "Enduron Lead"},
  {Type::everfreeze_ore, "Everfreeze Ore"},
  {Type::foucaultium, "Foucaultium"},
  {Type::lionbone_bort, "Lionbone Bort"},
  {Type::infernium, "Infernium"},
  {Type::boiled_egg_ore, "Boiled-Egg Ore"},
  {Type::marine_rutile, "Marine Rutile"},
  {Type::dawnstone, "Dawnstone"},
  {Type::cimmerian_cinnabar, "Cimmerian Cinnabar"},
  {Type::ouroboros_crystal, "Ouroboros Crystal"},
  {Type::parhelion_platinum, "Parhelion Platinum"},
  {Type::bonjelium, "Bonjelium"}
};

