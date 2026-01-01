/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugdialogtest.h"

#include "../akonadisearchdebugdialog.h"
#include "../akonadisearchdebugwidget.h"
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTest>

using namespace Qt::Literals::StringLiterals;
AkonadiSearchDebugDialogTest::AkonadiSearchDebugDialogTest(QObject *parent)
    : QObject(parent)
{
}

AkonadiSearchDebugDialogTest::~AkonadiSearchDebugDialogTest() = default;

void AkonadiSearchDebugDialogTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void AkonadiSearchDebugDialogTest::shouldHaveDefaultValue()
{
    Akonadi::Search::AkonadiSearchDebugDialog dlg;
    auto debugWidget = dlg.findChild<Akonadi::Search::AkonadiSearchDebugWidget *>(u"akonadisearchdebugwidget"_s);
    QVERIFY(debugWidget);
    auto editorWidget = debugWidget->findChild<QPlainTextEdit *>(u"plaintexteditor"_s);
    QVERIFY(editorWidget);
    auto lineEdit = debugWidget->findChild<QLineEdit *>(u"lineedit"_s);
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

void AkonadiSearchDebugDialogTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    Akonadi::Search::AkonadiSearchDebugDialog dlg;
    auto debugWidget = dlg.findChild<Akonadi::Search::AkonadiSearchDebugWidget *>(u"akonadisearchdebugwidget"_s);
    QVERIFY(debugWidget);
    auto lineEdit = debugWidget->findChild<QLineEdit *>(u"lineedit"_s);
    QVERIFY(lineEdit);
    const int value = 42;
    const QString akonadiItem = QString::number(value);
    dlg.setAkonadiId(value);
    QCOMPARE(lineEdit->text(), akonadiItem);
}

QTEST_MAIN(AkonadiSearchDebugDialogTest)

#include "moc_akonadisearchdebugdialogtest.cpp"
