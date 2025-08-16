#include "inventory_model.h"
#include <QBrush>
#include "probe_ui.h"

const std::vector<std::string> display_probes{
  // Don't show basic probes in this list.
  "M1", "M2", "M3", "M4", "M5", "M6", "M7", "M8", "M9",
  "M10", "R1", "R2", "R3", "R4", "R5", "R6", "B1", "B2",
  "D", "S", "C",
};

InventoryModel::InventoryModel(Options* options, const Layout* layout, QObject* parent): QAbstractTableModel(parent),
  options_(options), layout_(layout) {
  calculate_used_probes();
}

int InventoryModel::columnCount(const QModelIndex&) const {
  return column_count;
}

QVariant InventoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
  const auto col = static_cast<Column>(section);
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole) {
      switch (col) {
        case Column::Name:
          return tr("Name");
        case Column::Quantity:
          return tr("#");
        case Column::Used:
          return tr("Used");
        case Column::Remaining:
          return tr("Remaining");
      }
    }
  }
  return {};
}

int InventoryModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(display_probes.size());
}

QVariant InventoryModel::data(const QModelIndex& index, int role) const {
  const auto col = static_cast<Column>(index.column());
  const auto& probe_shorthand = display_probes.at(index.row());
  const auto probe_ix = Probe::idx_for_shorthand.at(probe_shorthand);
  const auto& probe = Probe::probes.at(probe_ix);
  const auto quantity = options_->get_probe_quantities().at(probe_ix);

  if (role == Qt::DisplayRole) {
    if (col == Column::Name) {
      return probe_display_name(&probe);
    }
    else if (col == Column::Quantity) {
      return quantity;
    }
    else if (col == Column::Used) {
      return used_probes_.at(probe_ix);
    }
    else if (col == Column::Remaining) {
      return static_cast<int>(quantity) - used_probes_.at(probe_ix);
    }
  }
  else if (role == Qt::EditRole) {
    if (col == Column::Quantity) {
      return quantity;
    }
  }
  else if (role == Qt::BackgroundRole) {
    if (used_probes_.at(probe_ix) > static_cast<int>(quantity)) {
      return QBrush(QColorConstants::Red);
    }
    else {
      return {};
    }
  }

  return {};
}

bool InventoryModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  bool success = false;
  const auto col = static_cast<Column>(index.column());
  const auto& probe_shorthand = display_probes.at(index.row());
  const auto probe_ix = Probe::idx_for_shorthand.at(probe_shorthand);

  if (role == Qt::EditRole) {
    if (col == Column::Quantity) {
      const auto val = value.toUInt(&success);
      if (success) {
        options_->mutable_probe_quantities()[probe_ix] = val;
      }
    }
  }

  if (success) {
    // Mark entire row as changed since calculated columns will change as well.
    calculate_used_probes();
    Q_EMIT(dataChanged(createIndex(index.row(), 0), createIndex(index.row(), column_count - 1)));
  }

  return success;
}

Qt::ItemFlags InventoryModel::flags(const QModelIndex& index) const {
  const auto col = static_cast<Column>(index.column());
  const auto defaults = Qt::ItemNeverHasChildren;
  switch (col) {
    case Column::Name:
      return defaults | Qt::ItemIsEnabled;
    case Column::Quantity:
      return defaults | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    case Column::Used:
    case Column::Remaining:
      return defaults;
  }

  return QAbstractTableModel::flags(index);
}

void InventoryModel::reset() {
  beginResetModel();
  calculate_used_probes();
  endResetModel();
}

void InventoryModel::calculate_used_probes() {
  for (uint32_t probe_ix = 0; probe_ix < Probe::num_probes; ++probe_ix) {
    const auto probe_id = Probe::probes.at(probe_ix).probe_id;
    used_probes_[probe_ix] = static_cast<unsigned int>(std::ranges::count_if(
      layout_->get_placements(), [probe_id](const Placement& placement) {
        return probe_id == placement.get_probe().probe_id;
      }));
  }
}
