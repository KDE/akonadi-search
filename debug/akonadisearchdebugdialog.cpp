/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cerrno>

#include "akonadisearchdebugdialog.h"
#include "akonadisearchdebugwidget.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Akonadi::Search;

class Akonadi::Search::AkonadiSearchDebugDialogPrivate
{
public:
    AkonadiSearchDebugWidget *mAkonadiSearchDebugWidget = nullptr;
};

AkonadiSearchDebugDialog::AkonadiSearchDebugDialog(QWidget *parent)
    : QDialog(parent)
    , d(new Akonadi::Search::AkonadiSearchDebugDialogPrivate)
{
    // Don't translate it's just a dialog to debug
    setWindowTitle(QStringLiteral("Debug Akonadi Search"));

    auto mainLayout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    // Don't translate it.
    user1Button->setText(QStringLiteral("Save As..."));
    connect(user1Button, &QPushButton::clicked, this, &AkonadiSearchDebugDialog::slotSaveAs);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AkonadiSearchDebugDialog::reject);
    d->mAkonadiSearchDebugWidget = new AkonadiSearchDebugWidget(this);
    d->mAkonadiSearchDebugWidget->setObjectName(QStringLiteral("akonadisearchdebugwidget"));
    mainLayout->addWidget(d->mAkonadiSearchDebugWidget);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

AkonadiSearchDebugDialog::~AkonadiSearchDebugDialog()
{
    writeConfig();
    delete d;
}

void AkonadiSearchDebugDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), "AkonadiSearchDebugDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void AkonadiSearchDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), "AkonadiSearchDebugDialog");
    group.writeEntry("Size", size());
}

void AkonadiSearchDebugDialog::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    d->mAkonadiSearchDebugWidget->setAkonadiId(akonadiId);
}

void AkonadiSearchDebugDialog::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    d->mAkonadiSearchDebugWidget->setSearchType(type);
}

void AkonadiSearchDebugDialog::doSearch()
{
    d->mAkonadiSearchDebugWidget->doSearch();
}

void AkonadiSearchDebugDialog::slotSaveAs()
{
    const QString filter = i18n("Text Files (*.txt);;All Files (*)");
    saveTextAs(d->mAkonadiSearchDebugWidget->plainText(), filter);
}

void AkonadiSearchDebugDialog::saveTextAs(const QString &text, const QString &filter)
{
    QPointer<QFileDialog> fdlg(new QFileDialog(this, QString(), QString(), filter));
    fdlg->setAcceptMode(QFileDialog::AcceptSave);
    if (fdlg->exec() == QDialog::Accepted && fdlg) {
        const QString fileName = fdlg->selectedFiles().at(0);
        if (!saveToFile(fileName, text)) {
            KMessageBox::error(this,
                               i18n("Could not write the file %1:\n"
                                    "\"%2\" is the detailed error description.",
                                    fileName,
                                    QString::fromLocal8Bit(strerror(errno))),
                               i18n("Save File Error"));
        }
    }
    delete fdlg;
}

bool AkonadiSearchDebugDialog::saveToFile(const QString &filename, const QString &text)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << text;
    file.close();
    return true;
}
