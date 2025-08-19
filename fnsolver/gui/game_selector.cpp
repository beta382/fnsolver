#include "game_selector.h"

GameSelectorWidget::GameSelectorWidget(QWidget* parent): QComboBox(parent) {
  addItems({
    tr("Original"),
    tr("Definitive"),
  });
  connect(this, &QComboBox::currentIndexChanged, [this]() {
    Q_EMIT(selected_game_changed(get_selected_game()));
  });
}

game::Version GameSelectorWidget::get_selected_game() const {
  return static_cast<game::Version>(currentIndex());
}

void GameSelectorWidget::set_selected_game(game::Version game) {
  setCurrentIndex(static_cast<int>(game));
}
