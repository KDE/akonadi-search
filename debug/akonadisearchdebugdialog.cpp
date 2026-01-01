/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cerrno>

#include "akonadisearchdebugdialog.h"
#include "akonadisearchdebugwidget.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search;

class Akonadi::Search::AkonadiSearchDebugDialogPrivate
{
public:
    AkonadiSearchDebugWidget *mAkonadiSearchDebugWidget = nullptr;
};

namespace
{
static const char myAkonadiSearchDebugDialogConfigGroupName[] = "AkonadiSearchDebugDialog";
}
AkonadiSearchDebugDialog::AkonadiSearchDebugDialog(QWidget *parent)
    : QDialog(parent)
    , d(new Akonadi::Search::AkonadiSearchDebugDialogPrivate)
{
    // Don't translate it's just a dialog to debug
    setWindowTitle(u"Debug Akonadi Search"_s);

    auto mainLayout = new QVBoxLayout(this);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    // Don't translate it.
    user1Button->setText(u"Save As..."_s);
    connect(user1Button, &QPushButton::clicked, this, &AkonadiSearchDebugDialog::slotSaveAs);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AkonadiSearchDebugDialog::reject);
    d->mAkonadiSearchDebugWidget = new AkonadiSearchDebugWidget(this);
    d->mAkonadiSearchDebugWidget->setObjectName("akonadisearchdebugwidget"_L1);
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
    create(); // ensure a window is created
    windowHandle()->resize(QSize(800, 600));
    KConfigGroup group(KSharedConfig::openStateConfig(), QLatin1StringView(myAkonadiSearchDebugDialogConfigGroupName));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584
}

void AkonadiSearchDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), QLatin1StringView(myAkonadiSearchDebugDialogConfigGroupName));
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.sync();
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
                               i18nc("@title:window", "Save File Error"));
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
    out << text;
    file.close();
    return true;
}

#include "moc_akonadisearchdebugdialog.cpp"
