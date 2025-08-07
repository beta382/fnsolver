#ifndef FNSOLVER_GUI_QOBJECTDELETER_H
#define FNSOLVER_GUI_QOBJECTDELETER_H
#include <QObject>

/**
 * Call deleteLater() on QObject instances.
 */
struct QObjectDeleter {
  void operator()(QObject* p) const {
    if (p != nullptr) {
      p->deleteLater();
    }
  }
};

#endif //FNSOLVER_GUI_QOBJECTDELETER_H
