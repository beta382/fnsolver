#include "image_provider.h"
#include <QPixmap>
#include <fnsolver/data/probe.h>

const QPixmap& ImageProvider::probe_image(const Probe* probe) const {
  // This needs to be inside the function definition to ensure resources are initialized before trying to get files.
  static const std::unordered_map<game::Version, std::unordered_map<Probe::Type, QPixmap>> probe_images{
    {
      game::Version::Original, {
        {Probe::Type::none, QPixmap(":/probe_icons/og/notvisited.png")},
        {Probe::Type::basic, QPixmap(":/probe_icons/og/basic.png")},
        {Probe::Type::mining, QPixmap(":/probe_icons/og/mining.png")},
        {Probe::Type::research, QPixmap(":/probe_icons/og/research.png")},
        {Probe::Type::booster, QPixmap(":/probe_icons/og/booster.png")},
        {Probe::Type::storage, QPixmap(":/probe_icons/og/storage.png")},
        {Probe::Type::duplicator, QPixmap(":/probe_icons/og/duplicator.png")},
        {Probe::Type::battle, QPixmap(":/probe_icons/og/battle.png")},
      }
    },
    {
      game::Version::Definitive, {
        {Probe::Type::none, QPixmap(":/probe_icons/de/notvisited.png")},
        {Probe::Type::basic, QPixmap(":/probe_icons/de/basic.png")},
        {Probe::Type::mining, QPixmap(":/probe_icons/de/mining.png")},
        {Probe::Type::research, QPixmap(":/probe_icons/de/research.png")},
        {Probe::Type::booster, QPixmap(":/probe_icons/de/booster.png")},
        {Probe::Type::storage, QPixmap(":/probe_icons/de/storage.png")},
        {Probe::Type::duplicator, QPixmap(":/probe_icons/de/duplicator.png")},
        // TODO: If the battle probes are ever separated out, use icons for each type.
        {Probe::Type::battle, QPixmap(":/probe_icons/de/melee.png")},
      }
    },
  };
  return probe_images.at(game_version_).at(probe->probe_type);
}
