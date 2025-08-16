#include <fnsolver/data/precious_resource.h>

#include <string>
#include <unordered_map>

#include "fnsite.h"

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

const std::unordered_map<precious_resource::Type, std::string> precious_resource::str_for_type = []() {
  std::unordered_map<Type, std::string> rev_map;
  rev_map.reserve(type_for_str.size());
  for (const auto& [a,b] : type_for_str) {
    rev_map.emplace(b, a);
  }

  return rev_map;
}();

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

unsigned int precious_resource::max_resource_quantity(Type precious_resource) {
  // Init inside function to workaround static initialization order.
  static const std::unordered_map<Type, unsigned int> max_resource_quantities = []() {
    std::unordered_map<Type, unsigned int> map;
    map.reserve(count);
    for (std::size_t resource_ix = 0; resource_ix < count; ++resource_ix) {
      const auto resource = static_cast<Type>(resource_ix);
      unsigned int max_quantity = 0;
      for (const FnSite& site : FnSite::sites) {
        max_quantity += site.precious_resource_quantities.at(resource_ix);
      }
      map.emplace(resource, max_quantity);
    }

    return map;
  }();

  return max_resource_quantities.at(precious_resource);
}
