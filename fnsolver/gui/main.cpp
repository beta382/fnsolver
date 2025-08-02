#include <QApplication>
#include <QIcon>
#include <QStyleFactory>

#include "main_window.h"
#include "fnsolver/fnsolver_config.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(config::kProjectOrganizationName);
  app.setApplicationName(config::kProjectName);
  app.setApplicationDisplayName(config::kProjectDisplayName);
  app.setApplicationVersion(config::kProjectVersion);
  app.setWindowIcon(QIcon(":/dataprobe.png"));

#ifdef Q_OS_WINDOWS
  // Using fusion style enables dark-mode detection on Windows.
  auto *style = QStyleFactory::create("fusion");
  if (style != nullptr) {
    app.setStyle(style);
  }
#endif

  MainWindow main_window;
  main_window.show();

  return app.exec();
}
