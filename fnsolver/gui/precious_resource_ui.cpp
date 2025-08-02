#include "precious_resource_ui.h"
#include <unordered_map>
#include <QApplication>

QString precious_resource_display_name(precious_resource::Type precious_resource) {
  // This needs to be inside the function definition to ensure translation is initialized before trying to get text.
  static const std::unordered_map<precious_resource::Type, QString> precious_resource_display_names{
    {precious_resource::Type::arc_sand_ore, qApp->translate("precious_resource_ui", "Arc Sand Ore")},
    {precious_resource::Type::aurorite, qApp->translate("precious_resource_ui", "Aurorite")},
    {precious_resource::Type::white_cometite, qApp->translate("precious_resource_ui", "White Cometite")},
    {precious_resource::Type::enduron_lead, qApp->translate("precious_resource_ui", "Enduron Lead")},
    {precious_resource::Type::everfreeze_ore, qApp->translate("precious_resource_ui", "Everfreeze Ore")},
    {precious_resource::Type::foucaultium, qApp->translate("precious_resource_ui", "Foucaultium")},
    {precious_resource::Type::lionbone_bort, qApp->translate("precious_resource_ui", "Lionbone Bort")},
    {precious_resource::Type::infernium, qApp->translate("precious_resource_ui", "Infernium")},
    {precious_resource::Type::boiled_egg_ore, qApp->translate("precious_resource_ui", "Boiled-Egg Ore")},
    {precious_resource::Type::marine_rutile, qApp->translate("precious_resource_ui", "Marine Rutile")},
    {precious_resource::Type::dawnstone, qApp->translate("precious_resource_ui", "Dawnstone")},
    {precious_resource::Type::cimmerian_cinnabar, qApp->translate("precious_resource_ui", "Cimmerian Cinnabar")},
    {precious_resource::Type::ouroboros_crystal, qApp->translate("precious_resource_ui", "Ouroboros Crystal")},
    {precious_resource::Type::parhelion_platinum, qApp->translate("precious_resource_ui", "Parhelion Platinum")},
    {precious_resource::Type::bonjelium, qApp->translate("precious_resource_ui", "Bonjelium")}
  };
  return precious_resource_display_names.at(precious_resource);
}
