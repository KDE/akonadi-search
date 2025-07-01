/*
  SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugwidgettest.h"
using namespace Qt::Literals::StringLiterals;

#include "../akonadisearchdebugsearchpathcombobox.h"
#include "../akonadisearchdebugwidget.h"
#include <KLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTest>

AkonadiSearchDebugWidgetTest::AkonadiSearchDebugWidgetTest(QObject *parent)
    : QObject(parent)
{
}

AkonadiSearchDebugWidgetTest::~AkonadiSearchDebugWidgetTest() = default;

void AkonadiSearchDebugWidgetTest::shouldHaveDefaultValue()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    auto button = widget.findChild<QPushButton *>(u"searchbutton"_s);
    QVERIFY(button);
    QVERIFY(!button->isEnabled());
    auto lineEdit = widget.findChild<KLineEdit *>(u"lineedit"_s);
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
    QVERIFY(lineEdit->trapReturnKey());
    QVERIFY(lineEdit->isClearButtonEnabled());
    auto editorWidget = widget.findChild<QPlainTextEdit *>(u"plaintexteditor"_s);
    QVERIFY(editorWidget->isReadOnly());
    QVERIFY(editorWidget);
    QVERIFY(editorWidget->toPlainText().isEmpty());
    auto searchCombo = widget.findChild<Akonadi::Search::AkonadiSearchDebugSearchPathComboBox *>(u"searchpathcombo"_s);
    QVERIFY(searchCombo);
}

void AkonadiSearchDebugWidgetTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    auto lineEdit = widget.findChild<KLineEdit *>(u"lineedit"_s);
    const int value = 42;
    const QString akonadiItem = QString::number(value);
    widget.setAkonadiId(value);
    QCOMPARE(lineEdit->text(), akonadiItem);
}

void AkonadiSearchDebugWidgetTest::shouldEnabledPushButtonWhenLineEditIsNotEmpty()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    const int value = 42;
    widget.setAkonadiId(value);
    auto button = widget.findChild<QPushButton *>(u"searchbutton"_s);
    QVERIFY(button->isEnabled());

    auto lineEdit = widget.findChild<KLineEdit *>(u"lineedit"_s);
    lineEdit->setText(QString());
    QVERIFY(!button->isEnabled());

    // trimmed string
    lineEdit->setText(u" "_s);
    QVERIFY(!button->isEnabled());
}

void AkonadiSearchDebugWidgetTest::shouldChangeSearchType()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::SearchType type = Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::Emails;
    widget.setSearchType(type);
    auto searchCombo = widget.findChild<Akonadi::Search::AkonadiSearchDebugSearchPathComboBox *>(u"searchpathcombo"_s);
    const QString path = searchCombo->pathFromEnum(type);
    QCOMPARE(searchCombo->searchPath(), path);
}

QTEST_MAIN(AkonadiSearchDebugWidgetTest)

#include "moc_akonadisearchdebugwidgettest.cpp"
