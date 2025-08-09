#include "description_widget.h"

#include <QVBoxLayout>

DescriptionWidget::DescriptionWidget(QWidget* parent): QWidget(parent), label_(new QLabel(this)) {
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(label_);
  layout->setContentsMargins(0, 0, 0, 0);
  label_->setTextFormat(Qt::MarkdownText);
  label_->setWordWrap(true);
  label_->setOpenExternalLinks(true);
  label_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void DescriptionWidget::set_text(const QString& text) {
  label_->setText(text);
}

void set_markdown_tooltip(QWidget* widget, const QString& text) {
  if (widget == nullptr) {
    return;
  }

  QTextDocument doc;
  doc.setMarkdown(text);
  widget->setToolTip(doc.toHtml());
}
