#ifndef FNSOLVER_GUI_SOLVER_PARAMS_WIDGET_H
#define FNSOLVER_GUI_SOLVER_PARAMS_WIDGET_H

#include <QCheckBox>
#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "abstract_solver_options_widget.h"
#include "fnsolver/solver/options.h"

class SolverParamsWidget : public AbstractSolverOptionsWidget {
  Q_OBJECT

public:
  explicit SolverParamsWidget(const Options* solver_options, QWidget* parent = nullptr);

  void apply_to_options(Options* options) const override;
  bool get_seed() const;

private:
  struct Widgets {
    QCheckBox* seed;
    QCheckBox* force_seed;
    QSpinBox* iterations;
    QSpinBox* bonus_iterations;
    QSpinBox* population;
    QSpinBox* offspring;
    QDoubleSpinBox* mutation_rate;
    QSpinBox* max_age;
    QSpinBox* threads;
  };

  Widgets widgets_;

private Q_SLOTS:
  void use_default_threads();
};


#endif //FNSOLVER_GUI_SOLVER_PARAMS_WIDGET_H
