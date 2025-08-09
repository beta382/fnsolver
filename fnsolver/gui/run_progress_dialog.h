#ifndef FNSOLVER_GUI_RUN_PROGRESS_DIALOG_H
#define FNSOLVER_GUI_RUN_PROGRESS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>

#include "main_window.h"
#include "solver_runner.h"
#include "fnsolver/solver/options.h"

class RunProgressDialog : public QDialog {
  Q_OBJECT

public:
  explicit RunProgressDialog(const Options& solver_options, QWidget* parent = nullptr);

  struct Widgets {
    QLabel* iteration;
    QLabel* time_elapsed;
    QLabel* time_remaining;
    QLabel* best_score;
    QLabel* killed;
    QLabel* last_improvement;
    QLabel* mining;
    QLabel* revenue;
    QLabel* storage;
    QProgressBar* progress_bar;
  };

  Widgets widgets_;
  Options solver_options_;
  SolverRunner* solver_runner_;
  QElapsedTimer solver_stopwatch_;

Q_SIGNALS:
  void solved(Layout layout);

private Q_SLOTS:
  void progress(const Solver::IterationStatus& iteration_status);
};


#endif //FNSOLVER_GUI_RUN_PROGRESS_DIALOG_H
