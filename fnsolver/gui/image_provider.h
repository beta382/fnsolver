#ifndef FNSOLVER_GUI_IMAGE_PROVIDER_H
#define FNSOLVER_GUI_IMAGE_PROVIDER_H

#include <QObject>
#include <QPixmap>
#include <QSvgRenderer>
#include "game.h"

class Probe;

class ImageProvider : public QObject {
  Q_OBJECT

public:
  using QObject::QObject;

  [[nodiscard]] game::Version game_version() const { return game_version_; }

  void set_game_version(const game::Version version) { game_version_ = version; }

  /**
   * Get loaded QPixmap for @p probe.
   */
  const QPixmap& probe_image(const Probe* probe) const;

  /**
   * Get loaded QSvgRenderer for @p probe level.
   */
  std::unique_ptr<QSvgRenderer> probe_level(const Probe* probe) const;

private:
  game::Version game_version_ = game::Version::Original;
};


#endif //FNSOLVER_GUI_IMAGE_PROVIDER_H
