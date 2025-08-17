#ifndef FNSOLVER_GUI_GAME_SELECTOR_H
#define FNSOLVER_GUI_GAME_SELECTOR_H

#include <QComboBox>
#include "game.h"

class GameSelectorWidget : public QComboBox {
  Q_OBJECT

public:
  explicit GameSelectorWidget(QWidget* parent = nullptr);
  Game get_selected_game() const;
  void set_selected_game(Game game);

Q_SIGNALS:
  void selected_game_changed(Game game);
};


#endif //FNSOLVER_GUI_GAME_SELECTOR_H
