#ifndef FNSOLVER_GUI_SOLUTION_WIDGET_H
#define FNSOLVER_GUI_SOLUTION_WIDGET_H

#include <QLabel>
#include <QWidget>
#include "fnsolver/layout/layout.h"

class SolutionWidget : public QWidget {
  Q_OBJECT

public:
  explicit SolutionWidget(QWidget* parent = nullptr);
  void set_layout(const Layout& layout);

private:
  QLabel* production_label_;
  QLabel* revenue_label_;
  QLabel* storage_label_;
  QLabel* resources_label_;
};


#endif //FNSOLVER_GUI_SOLUTION_WIDGET_H
