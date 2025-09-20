#include "probe_ui.h"
#include <QApplication>
#include <unordered_map>

QString probe_display_name(const Probe* probe) {
  // This needs to be inside the function definition to ensure translation is initialized before trying to get text.
  static const std::unordered_map<std::string, QString> probe_display_names{
    {"X", qApp->translate("probe_ui", "Locked")},
    {"-", qApp->translate("probe_ui", "Basic")},
    {"M1", qApp->translate("probe_ui", "Mining G1")},
    {"M2", qApp->translate("probe_ui", "Mining G2")},
    {"M3", qApp->translate("probe_ui", "Mining G3")},
    {"M4", qApp->translate("probe_ui", "Mining G4")},
    {"M5", qApp->translate("probe_ui", "Mining G5")},
    {"M6", qApp->translate("probe_ui", "Mining G6")},
    {"M7", qApp->translate("probe_ui", "Mining G7")},
    {"M8", qApp->translate("probe_ui", "Mining G8")},
    {"M9", qApp->translate("probe_ui", "Mining G9")},
    {"M10", qApp->translate("probe_ui", "Mining G10")},
    {"R1", qApp->translate("probe_ui", "Research G1")},
    {"R2", qApp->translate("probe_ui", "Research G2")},
    {"R3", qApp->translate("probe_ui", "Research G3")},
    {"R4", qApp->translate("probe_ui", "Research G4")},
    {"R5", qApp->translate("probe_ui", "Research G5")},
    {"R6", qApp->translate("probe_ui", "Research G6")},
    {"B1", qApp->translate("probe_ui", "Booster G1")},
    {"B2", qApp->translate("probe_ui", "Booster G2")},
    {"D", qApp->translate("probe_ui", "Duplicator")},
    {"S", qApp->translate("probe_ui", "Storage")},
    {"C", qApp->translate("probe_ui", "Combat")}
  };
  return probe_display_names.at(probe->shorthand);
}

const QPixmap& probe_image(const Probe* probe) {
  // This needs to be inside the function definition to ensure resources are initialized before trying to get files.
  static const std::unordered_map<Probe::Type, QPixmap> probe_images{
    {Probe::Type::none, QPixmap(":/probe_icons/og/notvisited.png")},
    {Probe::Type::basic, QPixmap(":/probe_icons/og/basic.png")},
    {Probe::Type::mining, QPixmap(":/probe_icons/og/mining.png")},
    {Probe::Type::research, QPixmap(":/probe_icons/og/research.png")},
    {Probe::Type::booster, QPixmap(":/probe_icons/og/booster.png")},
    {Probe::Type::storage, QPixmap(":/probe_icons/og/storage.png")},
    {Probe::Type::duplicator, QPixmap(":/probe_icons/og/duplicator.png")},
    {Probe::Type::battle, QPixmap(":/probe_icons/og/battle.png")},
  };
  return probe_images.at(probe->probe_type);
}
