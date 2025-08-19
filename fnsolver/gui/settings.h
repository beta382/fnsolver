#ifndef FNSOLVER_GUI_SETTINGS_H
#define FNSOLVER_GUI_SETTINGS_H

#include <QSettings>
#include <QStandardPaths>

#define FN_SETTING(T, name, defaultValue)                                      \
  [[nodiscard]] T get_##name() {                                               \
    return settings::detail::get_setting<T>(#name, defaultValue);              \
  }                                                                            \
  void set_##name(T value) {                                                   \
    settings::detail::set_setting(#name, std::forward<T>(value));              \
  }                                                                            \
  T default_##name() { return defaultValue; }

namespace settings {
namespace detail {
template <typename T>
T get_setting(const char* name, T default_value) {
  QSettings settings;
  return settings.value(name, std::forward<T>(default_value))
                 .template value<T>();
}

template <typename T>
void set_setting(const char* name, T value) {
  QSettings settings;
  settings.setValue(name, std::forward<T>(value));
}
} // namespace detail

FN_SETTING(QByteArray, main_window_geometry, {});

FN_SETTING(QString, last_file_dialog_path,
           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

FN_SETTING(qsizetype, max_recent_documents, 4);

FN_SETTING(QStringList, recent_documents, {});

FN_SETTING(int, game_version, 0);
} // namespace settings

#undef FN_SETTING

#endif // FNSOLVER_GUI_SETTINGS_H
