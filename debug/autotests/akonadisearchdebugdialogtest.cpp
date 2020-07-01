/*
  SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
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
