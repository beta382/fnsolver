#include "main_window.h"

#include <iostream>
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
#include <QHeaderView>
#include <QDockWidget>
#include <ranges>

#include "about_dialog.h"
#include "options_loader.h"
#include "run_dialog.h"
#include "settings.h"
#include "fnsolver/fnsolver_config.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), solver_options_(options_loader::default_options()),
  layout_(fill_layout(solver_options_.get_seed(), solver_options_.get_locked_sites())) {
  update_seed();
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
  menuFile->addAction(actions.file_new);
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
  toolbar->addAction(actions.file_new);
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
  widgets_.mira_map = new MiraMap(&layout_, central);
  mapLayout->addWidget(widgets_.mira_map);
  // Forces recalculation of geometry, otherwise map is zoomed in a strange way at startup.
  widgets_.mira_map->show();
  connect(widgets_.mira_map, &MiraMap::site_probe_map_changed, this,
          &MainWindow::data_changed);
  connect(widgets_.mira_map, &MiraMap::site_probe_map_changed, this,
          &MainWindow::probe_map_changed);
  connect(actions.view_zoom_in, &QAction::triggered, widgets_.mira_map,
          &MiraMap::zoom_in);
  connect(actions.view_zoom_out, &QAction::triggered, widgets_.mira_map,
          &MiraMap::zoom_out);
  connect(actions.view_zoom_all, &QAction::triggered, widgets_.mira_map,
          &MiraMap::fit_all);

  // Config pane
  // Inventory
  inventory_model_ = new InventoryModel(&solver_options_, &layout_, this);
  widgets_.inventory_table = new QTableView(this);
  widgets_.inventory_table->setModel(inventory_model_);
  widgets_.inventory_table->verticalHeader()->hide();
  widgets_.inventory_table->resizeColumnsToContents();
  widgets_.inventory_table->setSizeAdjustPolicy(QTableView::AdjustToContents);
  widgets_.inventory_table->setSizePolicy(QSizePolicy::MinimumExpanding,
                                          QSizePolicy::Preferred);
  auto* inventory_dock_widget = new QDockWidget(tr("Inventory"), this);
  inventory_dock_widget->setFeatures(QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable);
  inventory_dock_widget->setWidget(widgets_.inventory_table);
  addDockWidget(Qt::RightDockWidgetArea, inventory_dock_widget);

  // Current solution
  auto* solution_scroll_area = new QScrollArea(this);
  widgets_.solution_widget = new SolutionWidget(solution_scroll_area);
  widgets_.solution_widget->setMinimumWidth(
    widgets_.inventory_table->minimumWidth());
  widgets_.solution_widget->set_layout(layout_);
  solution_scroll_area->setWidget(widgets_.solution_widget);
  solution_scroll_area->setWidgetResizable(true);
  solution_scroll_area->setSizeAdjustPolicy(QScrollArea::AdjustToContents);
  solution_scroll_area->setSizePolicy(QSizePolicy::MinimumExpanding,
                                      QSizePolicy::Preferred);
  auto* resultsDockWidget = new QDockWidget(tr("Results"), this);
  resultsDockWidget->setFeatures(QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable);
  resultsDockWidget->setWidget(solution_scroll_area);
  addDockWidget(Qt::RightDockWidgetArea, resultsDockWidget);
}

void MainWindow::init_actions() {
  // File
  // New
  actions.file_new = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), tr("&New"), this);;
  actions.file_new->setShortcut(QKeySequence(QKeySequence::New));
  connect(actions.file_new, &QAction::triggered, this, &MainWindow::file_new);
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
  try {
    solver_options_ = options_loader::load_from_file(path.toStdString());
    layout_ = fill_layout(solver_options_.get_seed(), solver_options_.get_locked_sites());
    update_seed();
    widgets_.mira_map->set_layout(&layout_);

    setWindowFilePath(path);
    setWindowModified(false);
    update_window_title();
    add_recent_document(path);
  }
  catch (const std::exception& e) {
    QMessageBox::critical(this, tr("Error opening file"),
                          tr("%1 is not valid.").arg(path));
  }
}

void MainWindow::save_to_path(const QString& path) {
  try {
    options_loader::save_to_file(path.toStdString(), solver_options_);
  }
  catch (const std::exception& e) {
    QMessageBox::critical(this, tr("Error saving file"),
                          tr("The file could not be opened for writing."));
    return;
  }

  setWindowFilePath(path);
  setWindowModified(false);
  update_window_title();
  add_recent_document(path);
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

void MainWindow::update_seed() {
  std::vector<Placement> locked_sites;
  std::vector<Placement> seed;
  for (const auto& placement : layout_.get_placements()) {
    if (placement.get_probe().probe_type == Probe::Type::none) {
      locked_sites.emplace_back(placement);
    }
    else if (placement.get_probe().probe_type != Probe::Type::basic) {
      seed.emplace_back(placement);
    }
  }
  solver_options_.set_locked_sites(locked_sites);
  solver_options_.set_seed(seed);
}

Layout MainWindow::fill_layout(std::vector<Placement> seed, std::vector<Placement> locked_sites) {
  // Start full of basic probes.
  std::unordered_map<FnSite::id_t, Placement> placements;
  for (const auto& site : FnSite::sites) {
    placements.emplace(site.site_id, Placement(site, Probe::probes.at(Probe::idx_for_shorthand.at("-"))));
  }

  // Apply seed and locked_sites.
  for (const auto& placement_list : {seed, locked_sites}) {
    for (const auto& placement : placement_list) {
      placements.insert_or_assign(placement.get_site().site_id, placement);
    }
  }

  // Layout ctor requires that placements is sorted.
  const auto placements_view = placements | std::views::values;
  std::vector<Placement> placements_sorted(placements_view.begin(), placements_view.end());
  std::ranges::sort(placements_sorted, [](const Placement& lhs, const Placement& rhs) {
    return lhs.get_site().site_id < rhs.get_site().site_id;
  });

  return {placements_sorted};
}

void MainWindow::file_new() {
  if (!safe_to_close_file()) {
    return;
  }

  solver_options_ = options_loader::default_options();
  layout_ = fill_layout(solver_options_.get_seed(), solver_options_.get_locked_sites());
  update_seed();
  widgets_.mira_map->set_layout(&layout_);

  setWindowFilePath({});
  setWindowModified(false);
  update_window_title();
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
  inventory_model_->reset();
  widgets_.solution_widget->set_layout(layout_);
  update_seed();
}

void MainWindow::solve() {
  auto* run_dialog = new RunDialog(&solver_options_, this);
  connect(run_dialog, &RunDialog::finished, [run_dialog]() { run_dialog->deleteLater(); });
  connect(run_dialog, &RunDialog::options_changed, this, &MainWindow::data_changed);
  // Doing this instead of exec() means we can track changes to run options while the dialog is open.
  run_dialog->setModal(true);
  run_dialog->show();
}
