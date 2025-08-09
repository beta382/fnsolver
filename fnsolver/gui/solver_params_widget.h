#ifndef FNSOLVER_GUI_SOLVER_PARAMS_WIDGET_H
#define FNSOLVER_GUI_SOLVER_PARAMS_WIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "fnsolver/solver/options.h"

class SolverParamsWidget : public QWidget {
  Q_OBJECT

public:
  explicit SolverParamsWidget(const Options* solver_options, QWidget* parent = nullptr);

  void apply_to_options(Options* options) const;

private:
  struct Widgets {
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
