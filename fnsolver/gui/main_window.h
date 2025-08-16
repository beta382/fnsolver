#ifndef FNSOLVER_GUI_MAIN_WINDOW_H
#define FNSOLVER_GUI_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTabBar>
#include <QAction>
#include <QElapsedTimer>
#include <QMenu>
#include <QTableView>
#include <QProgressDialog>

#include "inventory_model.h"
#include "mira_map.h"
#include "run_dialog.h"
#include "solution_widget.h"
#include "fnsolver/solver/options.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);

protected:
  void closeEvent(QCloseEvent* event) override;

private:
  struct Actions {
    QAction* file_new = nullptr;
    QAction* file_open = nullptr;
    QMenu* file_recent = nullptr;
    QAction* file_save = nullptr;
    QAction* file_save_as = nullptr;
    QAction* file_exit = nullptr;
    QAction* view_zoom_in = nullptr;
    QAction* view_zoom_out = nullptr;
    QAction* view_zoom_all = nullptr;
    QAction* layout_load_from_frontiernav = nullptr;
    QAction* layout_show_in_frontiernav = nullptr;
    QAction* layout_unlock_all = nullptr;
    QAction* layout_lock_all = nullptr;
    QAction* layout_set_all_basic = nullptr;
    QAction* help_about = nullptr;
    QAction* help_website = nullptr;
    QAction* run_simulation = nullptr;
  };

  Actions actions;

  struct Widgets {
    MiraMap* mira_map = nullptr;
    QTableView* inventory_table = nullptr;
    SolutionWidget* solution_widget = nullptr;
  };

  Widgets widgets_;
  InventoryModel* inventory_model_ = nullptr;
  Options solver_options_;
  Layout layout_;

  void init_ui();
  void init_actions();
  void update_window_title();
  void add_recent_document(const QString& path);
  void update_recent_documents();
  void open_from_path(const QString& path);
  void save_to_path(const QString& path);
  bool safe_to_close_file();
  void update_options_seed();
  void update_options_territories();
  /** Put a basic probe on any site not included in the layout. */
  static Layout fill_layout(std::vector<Placement> seed, std::vector<Placement> locked_sites);

private Q_SLOTS:
  void file_new();
  void file_open();
  void file_save();
  void file_save_as();
  void layout_show_in_frontiernav();
  void layout_load_from_frontiernav();
  void layout_unlock_all();
  void layout_lock_all();
  void layout_set_all_basic();
  void help_about();
  void help_website();
  void data_changed();
  void probe_map_changed();
  void options_changed(const Options& options);
  void solve();
  void solved(const Layout& layout);
};

#endif //FNSOLVER_GUI_MAIN_WINDOW_H
