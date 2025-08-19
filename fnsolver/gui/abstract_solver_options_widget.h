#ifndef FNSOLVER_GUI_ABSTRACT_SOLVER_OPTIONS_WIDGET_H
#define FNSOLVER_GUI_ABSTRACT_SOLVER_OPTIONS_WIDGET_H

#include <QWidget>
#include "fnsolver/solver/options.h"

/**
 * Base class for solver options pages.
 *
 * These pages are presented in RunDialog before running the solver.
 */
class AbstractSolverOptionsWidget : public QWidget {
  Q_OBJECT

public:
  using QWidget::QWidget;

  /**
   * Apply this page's options to @p options.
   * @param options
   */
  virtual void apply_to_options(Options* options) const = 0;
};


#endif //FNSOLVER_GUI_ABSTRACT_SOLVER_OPTIONS_WIDGET_H
