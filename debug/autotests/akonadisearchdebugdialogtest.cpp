/*
  Copyright (c) 2014-2020 Laurent Montel <montel@kde.org>

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

#include "akonadisearchdebugdialogtest.h"
#include "../akonadisearchdebugdialog.h"
#include "../akonadisearchdebugwidget.h"
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <KLineEdit>
#include <QTest>

AkonadiSearchDebugDialogTest::AkonadiSearchDebugDialogTest(QObject *parent)
    : QObject(parent)
{
}

AkonadiSearchDebugDialogTest::~AkonadiSearchDebugDialogTest()
{
}

void AkonadiSearchDebugDialogTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void AkonadiSearchDebugDialogTest::shouldHaveDefaultValue()
{
    Akonadi::Search::AkonadiSearchDebugDialog dlg;
    Akonadi::Search::AkonadiSearchDebugWidget *debugWidget = dlg.findChild<Akonadi::Search::AkonadiSearchDebugWidget *>(QStringLiteral("akonadisearchdebugwidget"));
    QVERIFY(debugWidget);
    QPlainTextEdit *editorWidget = debugWidget->findChild<QPlainTextEdit *>(QStringLiteral("plaintexteditor"));
    QVERIFY(editorWidget);
    KLineEdit *lineEdit = debugWidget->findChild<KLineEdit *>(QStringLiteral("lineedit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

void AkonadiSearchDebugDialogTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    Akonadi::Search::AkonadiSearchDebugDialog dlg;
    Akonadi::Search::AkonadiSearchDebugWidget *debugWidget = dlg.findChild<Akonadi::Search::AkonadiSearchDebugWidget *>(QStringLiteral("akonadisearchdebugwidget"));
    QVERIFY(debugWidget);
    KLineEdit *lineEdit = debugWidget->findChild<KLineEdit *>(QStringLiteral("lineedit"));
    QVERIFY(lineEdit);
    const int value = 42;
    const QString akonadiItem = QString::number(value);
    dlg.setAkonadiId(value);
    QCOMPARE(lineEdit->text(), akonadiItem);
}

QTEST_MAIN(AkonadiSearchDebugDialogTest)
