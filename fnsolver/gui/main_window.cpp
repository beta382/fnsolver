#include "main_window.h"
#include <thread>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QFileInfo>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QVBoxLayout>

#include "about_dialog.h"
#include "settings.h"
#include "fnsolver/fnsolver_config.h"

Options default_options() {
  return {
    false,
    ScoreFunction::create_max_effective_mining(2),
    ScoreFunction::create_max_mining(),
    {},
    {},
    {},
    {},
    false,
    {},
    0,
    0,
    0,
    1000,
    0,
    100,
    200,
    0.04,
    50,
    std::thread::hardware_concurrency()
  };
};

Layout default_layout() {
  std::vector<Placement> placements;
  placements.reserve(FnSite::num_sites);
  for (const auto& site : FnSite::sites) {
    placements.emplace_back(site, Probe::probes.at(Probe::idx_for_shorthand.at("-")));
  }
  return {placements};
}

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), solver_options_(default_options()),
  layout_(default_layout()) {
  init_ui();
}

void MainWindow::closeEvent(QCloseEvent* event) {
  if (!safe_to_close_file()) {
    event->ignore();
    return;
  }
  settings::set_main_window_geometry(saveGeometry());
  event->accept();
}

void MainWindow::init_ui() {
  if (!restoreGeometry(settings::get_main_window_geometry())) {
    resize(1024, 768);
  }
  setWindowTitle(QString("%1[*]").arg(qApp->applicationDisplayName()));

  init_actions();

  auto central = new QWidget(this);
  setCentralWidget(central);

  // Menubar
  // File menu
  auto* menuFile = menuBar()->addMenu(tr("&File"));
  menuFile->addAction(actions.file_open);
  menuFile->addMenu(actions.file_recent);
  menuFile->addAction(actions.file_save);
  menuFile->addAction(actions.file_save_as);
  menuFile->addSeparator();
  menuFile->addAction(actions.file_exit);
  // View menu
  auto* menuView = menuBar()->addMenu(tr("&View"));
  menuView->addAction(actions.view_zoom_in);
  menuView->addAction(actions.view_zoom_out);
  menuView->addAction(actions.view_zoom_all);
  // Help menu
  auto* menuHelp = menuBar()->addMenu(tr("&Help"));
  menuHelp->addAction(actions.help_about);
  menuHelp->addAction(actions.help_website);

  // Toolbar
  auto* toolbar = addToolBar(tr("Toolbar"));
  toolbar->setMovable(false);
  toolbar->addAction(actions.file_open);
  toolbar->addAction(actions.file_save);
  toolbar->addAction(actions.file_save_as);
  toolbar->addSeparator();
  toolbar->addAction(actions.view_zoom_in);
  toolbar->addAction(actions.view_zoom_out);
  toolbar->addAction(actions.view_zoom_all);
  toolbar->addSeparator();
  toolbar->addAction(actions.run_simulation);

  // Map
  auto mapLayout = new QVBoxLayout(central);
  widgets_.miraMap = new MiraMap(&layout_, central);
  mapLayout->addWidget(widgets_.miraMap);
  // Forces recalculation of geometry, otherwise map is zoomed in a strange way at startup.
  widgets_.miraMap->show();
  connect(widgets_.miraMap, &MiraMap::site_probe_map_changed, this,
          &MainWindow::data_changed);
  connect(widgets_.miraMap, &MiraMap::site_probe_map_changed, this,
          &MainWindow::probe_map_changed);
  connect(actions.view_zoom_in, &QAction::triggered, widgets_.miraMap,
          &MiraMap::zoom_in);
  connect(actions.view_zoom_out, &QAction::triggered, widgets_.miraMap,
          &MiraMap::zoom_out);
  connect(actions.view_zoom_all, &QAction::triggered, widgets_.miraMap,
          &MiraMap::fit_all);
}

void MainWindow::init_actions() {
  // File
  // Open
  actions.file_open =
    new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen), tr("&Open"), this);
  actions.file_open->setShortcut(QKeySequence::Open);
  connect(actions.file_open, &QAction::triggered, this, &MainWindow::file_open);
  // Recent
  actions.file_recent = new QMenu(tr("Recent Documents"), this);
  actions.file_recent->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpenRecent));
  update_recent_documents();
  // Save
  actions.file_save =
    new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), tr("&Save"), this);
  actions.file_save->setShortcut(QKeySequence::Save);
  connect(actions.file_save, &QAction::triggered, this, &MainWindow::file_save);
  // Save As
  actions.file_save_as = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs),
                                     tr("Save &As"), this);
  actions.file_save_as->setShortcut(QKeySequence::SaveAs);
  connect(actions.file_save_as, &QAction::triggered, this,
          &MainWindow::file_save_as);
  // Exit
  actions.file_exit = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit),
                                  tr("E&xit"), this);
  connect(actions.file_exit, &QAction::triggered, this, &MainWindow::close);

  // View
  // Zoom In
  actions.view_zoom_in =
    new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn), tr("Zoom &In"), this);
  actions.view_zoom_in->setShortcut(QKeySequence::ZoomIn);
  // Zoom Out
  actions.view_zoom_out =
    new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut), tr("Zoom &Out"), this);
  actions.view_zoom_out->setShortcut(QKeySequence::ZoomOut);
  // Zoom All
  actions.view_zoom_all = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ZoomFitBest),
                                      tr("Zoom &All"), this);
  actions.view_zoom_all->setShortcut(QKeySequence::SelectAll);

  // Help
  // About
  actions.help_about =
    new QAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout), tr("About"), this);
  connect(actions.help_about, &QAction::triggered, this, &MainWindow::help_about);
  // Show website
  actions.help_website =
    new QAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpFaq), tr("Website"), this);
  actions.help_website->setShortcut(QKeySequence::HelpContents);
  connect(actions.help_website, &QAction::triggered, this,
          &MainWindow::help_website);

  // Run Simulation
  actions.run_simulation = new QAction(
    QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart), tr("Solve"), this);
  actions.run_simulation->setShortcut(QKeySequence::Refresh);
  connect(actions.run_simulation, &QAction::triggered, this, &MainWindow::solve);
}

void MainWindow::update_window_title() {
  if (!windowFilePath().isEmpty()) {
    setWindowTitle(QString("%1[*]").arg(windowFilePath()));
  }
  else if (isWindowModified()) {
    setWindowTitle(tr("Unsaved file[*]"));
  }
  else {
    setWindowTitle({});
  }
}

void MainWindow::add_recent_document(const QString& path) {
  auto recent_paths = settings::get_recent_documents();
  recent_paths.prepend(path);
  settings::set_recent_documents(recent_paths);
  update_recent_documents();
}

void MainWindow::update_recent_documents() {
  auto recent_paths = settings::get_recent_documents();
  recent_paths.removeDuplicates();
  recent_paths.removeIf([](const QString& path) {
    QFileInfo fileInfo(path);
    return !fileInfo.exists();
  });
  recent_paths.resize(
    std::min(recent_paths.size(), settings::get_max_recent_documents()));
  actions.file_recent->setEnabled(!recent_paths.empty());
  actions.file_recent->clear();
  for (const auto& recentPath : recent_paths) {
    auto* actRecentPath = new QAction(recentPath, actions.file_recent);
    connect(actRecentPath, &QAction::triggered, [this, recentPath]() {
      if (safe_to_close_file()) {
        open_from_path(recentPath);
      }
    });
    actions.file_recent->addAction(actRecentPath);
  }
  settings::set_recent_documents(recent_paths);
}

void MainWindow::open_from_path(const QString& path) {
  // TODO: Implement.
}

void MainWindow::save_to_path(const QString& path) {
  // TODO: Implement.
}

bool MainWindow::safe_to_close_file() {
  if (isWindowModified()) {
    QMessageBox dialog(this);
    dialog.setIcon(QMessageBox::Question);
    dialog.setText(tr("This layout has been modified."));
    dialog.setInformativeText(tr("Would you like to save your changes?"));
    dialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
      QMessageBox::Cancel);
    dialog.setDefaultButton(QMessageBox::Save);
    const auto ret = dialog.exec();
    if (ret == QMessageBox::Save) {
      // Save, then close.
      file_save();
      if (isWindowModified()) {
        // User cancelled save box.
        return false;
      }
    }
    else if (ret == QMessageBox::Cancel) {
      // Don't close.
      return false;
    }
  }
  // Either no need to save or discard changes and close.

  return true;
}

void MainWindow::file_open() {
  if (!safe_to_close_file()) {
    return;
  }

  QFileDialog dialog(this, tr("Open Simulation..."));
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter(tr("Simulations (*.toml)"));
  dialog.setDirectory(settings::get_last_file_dialog_path());
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const auto filenames = dialog.selectedFiles();
  const QFileInfo file_info(filenames.at(0));
  open_from_path(file_info.filePath());
  settings::set_last_file_dialog_path(file_info.absoluteDir().path());
}

void MainWindow::file_save() {
  if (windowFilePath().isEmpty()) {
    file_save_as();
  }
  else {
    save_to_path(windowFilePath());
  }
}

void MainWindow::file_save_as() {
  QFileDialog dialog(this, tr("Save Simulation..."));
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setNameFilter(tr("Simulations (*.toml)"));
  if (!windowFilePath().isEmpty()) {
    dialog.selectFile(windowFilePath());
  }
  else {
    dialog.setDirectory(settings::get_last_file_dialog_path());
  }
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const auto filenames = dialog.selectedFiles();
  const QFileInfo file_info(filenames.at(0));
  save_to_path(file_info.filePath());
  settings::set_last_file_dialog_path(file_info.absoluteDir().path());
}

void MainWindow::help_about() {
  AboutDialog about(this);
  about.exec();
}

void MainWindow::help_website() {
  QDesktopServices::openUrl(
    QUrl(config::kProjectHomepageUrl, QUrl::TolerantMode));
}

void MainWindow::data_changed() {
  setWindowModified(true);
  update_window_title();
}

void MainWindow::probe_map_changed() {
  // TODO: Implement.
}

void MainWindow::solve() {
  // TODO: Implement.
}

void MainWindow::progress(unsigned long iter, double bestScore, double worstScore, unsigned long killed) {
  // TODO: Implement.
}
