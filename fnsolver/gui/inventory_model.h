#ifndef FNSOLVER_GUI_INVENTORY_MODEL_H
#define FNSOLVER_GUI_INVENTORY_MODEL_H

#include <QAbstractTableModel>

#include "game.h"
#include "fnsolver/solver/options.h"
#include "fnsolver/layout/layout.h"

class InventoryModel : public QAbstractTableModel {
  Q_OBJECT

public:
  enum class Column {
    Name,
    Quantity,
    Used,
    Remaining,
    Max,
  };

  static constexpr auto column_count = static_cast<int>(Column::Max) + 1;

  explicit InventoryModel(Options* options, const Layout* layout, const Game* game, QObject* parent = nullptr);
  [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index,
                              int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role) override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Recalculate probe counts.
   */
  void reset();

private:
  Options* options_;
  const Layout* layout_;
  const Game* game_;
  std::array<int, Probe::num_probes> used_probes_{};

  void calculate_used_probes();
};


#endif //FNSOLVER_GUI_INVENTORY_MODEL_H
