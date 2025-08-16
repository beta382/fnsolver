#ifndef FNSOLVER_GUI_RUN_DIALOG_H
#define FNSOLVER_GUI_RUN_DIALOG_H

#include <QDialog>

#include "constraints_widget.h"
#include "score_function_widget.h"
#include "solver_params_widget.h"
#include "fnsolver/solver/options.h"

class RunDialog : public QDialog {
  Q_OBJECT

public:
  explicit RunDialog(Options* solver_options, QWidget* parent = nullptr);

Q_SIGNALS:
  void options_changed(const Options& options);
  void solved(Layout layout);

private:
  struct Widgets {
    ScoreFunctionWidget* scorefunction = nullptr;
    ScoreFunctionWidget* tiebreaker = nullptr;
    ConstraintsWidget* constraints = nullptr;
    SolverParamsWidget* solver_params = nullptr;
  };

  Widgets widgets_;
  Options* solver_options_;

private Q_SLOTS:
  void solve();
};


#endif //FNSOLVER_GUI_RUN_DIALOG_H
