#include "UpdateDialog.hpp"

#include "Translator.hpp"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

UpdateDialog::UpdateDialog(const QString& currentVersion,
                           const QString& latestVersion,
                           const QString& url,
                           const QString& title,
                           const QString& notes,
                           QWidget* parent)
    : QDialog(parent), url_(url), latestVersion_(latestVersion)
{
    setWindowTitle(Lang::tr("updatedlg.title"));
    resize(520, 420);

    auto* layout = new QVBoxLayout(this);

    auto* heading = new QLabel(this);
    heading->setTextFormat(Qt::RichText);
    heading->setText(QStringLiteral("<h3>%1</h3>")
                         .arg(title.isEmpty() ? Lang::tr("updatedlg.new_version")
                                              : title.toHtmlEscaped()));
    layout->addWidget(heading);

    auto* versions = new QLabel(this);
    versions->setText(Lang::tr("updatedlg.versions")
                          .arg(currentVersion, latestVersion));
    layout->addWidget(versions);

    auto* notesView = new QTextBrowser(this);
    notesView->setOpenExternalLinks(true);
    if (notes.trimmed().isEmpty())
        notesView->setPlainText(Lang::tr("updatedlg.no_notes"));
    else
        notesView->setMarkdown(notes);
    layout->addWidget(notesView, 1);

    auto* buttons = new QDialogButtonBox(this);
    auto* downloadButton =
        buttons->addButton(Lang::tr("updatedlg.download"), QDialogButtonBox::AcceptRole);
    auto* skipButton =
        buttons->addButton(Lang::tr("updatedlg.skip"), QDialogButtonBox::DestructiveRole);
    buttons->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttons);

    downloadButton->setDefault(true);
    if (url_.isEmpty())
        downloadButton->setEnabled(false);

    connect(downloadButton, &QPushButton::clicked, this, [this]()
            {
                if (!url_.isEmpty())
                    QDesktopServices::openUrl(QUrl(url_));
                accept();
            });
    connect(skipButton, &QPushButton::clicked, this, [this]()
            {
                emit skipVersionRequested(latestVersion_);
                reject();
            });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
