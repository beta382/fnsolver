#ifndef FNSOLVER_GUI_SOLVER_RUNNER_H
#define FNSOLVER_GUI_SOLVER_RUNNER_H

#include <QThread>
#include "fnsolver/solver/options.h"
#include "fnsolver/solver/solver.h"

class SolverRunner : public QThread {
  Q_OBJECT

public:
  explicit SolverRunner(const Options& options, QObject* parent = nullptr);

Q_SIGNALS:
  void progress(Solver::IterationStatus iteration_status);
  void solved(Layout layout);

protected:
  void run() override;

private:
  Options solver_options_;
};


#endif //FNSOLVER_GUI_SOLVER_RUNNER_H
