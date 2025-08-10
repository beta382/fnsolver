#include <QApplication>
#include <QIcon>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>

#include "main_window.h"
#include "fnsolver/fnsolver_config.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(config::kProjectOrganizationName);
  app.setApplicationName(config::kProjectName);
  app.setApplicationDisplayName(config::kProjectDisplayName);
  app.setApplicationVersion(config::kProjectVersion);
  app.setWindowIcon(QIcon(":/dataprobe.png"));

  // Translations
  QTranslator qt_translator;
  if (qt_translator.load(QLocale::system(), "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
    app.installTranslator(&qt_translator);
  }
  QTranslator app_translator;
  if (app_translator.load(QLocale::system(), QString(":/i18n/%1_en_US.qm").arg(app.applicationName()))) {
    app.installTranslator(&app_translator);
  }

  MainWindow main_window;
  main_window.show();

  return app.exec();
}
