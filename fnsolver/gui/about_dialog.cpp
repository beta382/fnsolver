#include "about_dialog.h"
#include "fnsolver/fnsolver_config.h"
#include <QApplication>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);

  // Icon
  auto* icon_label = new QLabel(this);
  icon_label->setPixmap(QPixmap(":/dataprobe.png"));
  layout->addWidget(icon_label, 0, Qt::AlignCenter);

  // Title
  auto* title_label = new QLabel(
    QString("<b><big>%1</big></b>").arg(qApp->applicationDisplayName()),
    this);
  layout->addWidget(title_label, 0, Qt::AlignCenter);

  // Version
  auto* version_label = new QLabel(qApp->applicationVersion(), this);
  layout->addWidget(version_label, 0, Qt::AlignCenter);

  // Github
  auto* website_label = new QLabel(
    QString("<a href=\"%1\">%1</a>").arg(config::kProjectHomepageUrl), this);
  website_label->setOpenExternalLinks(true);
  layout->addWidget(website_label, 0, Qt::AlignCenter);

  // About Qt
  auto* about_qt_label =
    new QLabel(tr("<a href=\"aboutqt\">About Qt...</a>"), this);
  connect(about_qt_label, &QLabel::linkActivated, qApp, &QApplication::aboutQt);
  layout->addWidget(about_qt_label, 0, Qt::AlignCenter);

  // Close button
  auto* button_box = new QDialogButtonBox(QDialogButtonBox::Close, this);
  connect(button_box, &QDialogButtonBox::rejected, this, &AboutDialog::close);
  layout->addWidget(button_box);
}
