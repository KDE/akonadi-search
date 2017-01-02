/*
  Copyright (c) 2014-2017 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <errno.h>

#include "akonadisearchdebugdialog.h"
#include "akonadisearchdebugwidget.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QVBoxLayout>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPointer>
#include <QFileDialog>

using namespace Akonadi::Search;

class Akonadi::Search::AkonadiSearchDebugDialogPrivate
{
public:
    AkonadiSearchDebugDialogPrivate()
        : mAkonadiSearchDebugWidget(nullptr)
    {

    }

    AkonadiSearchDebugWidget *mAkonadiSearchDebugWidget;
};

AkonadiSearchDebugDialog::AkonadiSearchDebugDialog(QWidget *parent)
    : QDialog(parent),
      d(new Akonadi::Search::AkonadiSearchDebugDialogPrivate)
{
    //Don't translate it's just a dialog to debug
    setWindowTitle(QStringLiteral("Debug Akonadi Search"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    //Don't translate it.
    user1Button->setText(QStringLiteral("Save As..."));
    connect(user1Button, &QPushButton::clicked, this, &AkonadiSearchDebugDialog::slotSaveAs);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
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
    KConfigGroup group(KSharedConfig::openConfig(), "AkonadiSearchDebugDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void AkonadiSearchDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AkonadiSearchDebugDialog");
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
    out.setCodec("UTF-8");
    out << text;
    file.close();
    return true;
}
