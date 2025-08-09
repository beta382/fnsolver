#ifndef FNSOLVER_GUI_DESCRIPTION_WIDGET_H
#define FNSOLVER_GUI_DESCRIPTION_WIDGET_H

#include <QLabel>
#include <QWidget>

/**
 * Display a textual description.
 *
 * Text is in Markdown format.
 */
class DescriptionWidget : public QWidget {
  Q_OBJECT

public:
  explicit DescriptionWidget(QWidget* parent = nullptr);

  explicit DescriptionWidget(const QString& text, QWidget* parent = nullptr): DescriptionWidget(parent) {
    set_text(text);
  }

  void set_text(const QString& text);

private:
  QLabel* label_;
};

/**
 * Apply a tooltop written with Markdown to @p widget.
 *
 * @param widget
 * @param text
 */
void set_markdown_tooltip(QWidget* widget, const QString& text);

#endif //FNSOLVER_GUI_DESCRIPTION_WIDGET_H
