/*
  Copyright (c) 2014-2019 Montel Laurent <montel@kde.org>

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

#include "akonadisearchdebugwidgettest.h"
#include <QPlainTextEdit>
#include "../akonadisearchdebugwidget.h"
#include <qtest.h>
#include "../akonadisearchdebugsearchpathcombobox.h"
#include <KLineEdit>
#include <QPushButton>

AkonadiSearchDebugWidgetTest::AkonadiSearchDebugWidgetTest(QObject *parent)
    : QObject(parent)
{

}

AkonadiSearchDebugWidgetTest::~AkonadiSearchDebugWidgetTest()
{

}

void AkonadiSearchDebugWidgetTest::shouldHaveDefaultValue()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    QPushButton *button = widget.findChild<QPushButton *>(QStringLiteral("searchbutton"));
    QVERIFY(button);
    QVERIFY(!button->isEnabled());
    KLineEdit *lineEdit = widget.findChild<KLineEdit *>(QStringLiteral("lineedit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
    QVERIFY(lineEdit->trapReturnKey());
    QVERIFY(lineEdit->isClearButtonEnabled());
    QPlainTextEdit *editorWidget = widget.findChild<QPlainTextEdit *>(QStringLiteral("plaintexteditor"));
    QVERIFY(editorWidget->isReadOnly());
    QVERIFY(editorWidget);
    QVERIFY(editorWidget->toPlainText().isEmpty());
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox *searchCombo = widget.findChild<Akonadi::Search::AkonadiSearchDebugSearchPathComboBox *>(QStringLiteral("searchpathcombo"));
    QVERIFY(searchCombo);
}

void AkonadiSearchDebugWidgetTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    KLineEdit *lineEdit = widget.findChild<KLineEdit *>(QStringLiteral("lineedit"));
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
    QPushButton *button = widget.findChild<QPushButton *>(QStringLiteral("searchbutton"));
    QVERIFY(button->isEnabled());

    KLineEdit *lineEdit = widget.findChild<KLineEdit *>(QStringLiteral("lineedit"));
    lineEdit->setText(QString());
    QVERIFY(!button->isEnabled());

    //trimmed string
    lineEdit->setText(QStringLiteral(" "));
    QVERIFY(!button->isEnabled());

}

void AkonadiSearchDebugWidgetTest::shouldChangeSearchType()
{
    Akonadi::Search::AkonadiSearchDebugWidget widget;
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::SearchType type = Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::Emails;
    widget.setSearchType(type);
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox *searchCombo = widget.findChild<Akonadi::Search::AkonadiSearchDebugSearchPathComboBox *>(QStringLiteral("searchpathcombo"));
    const QString path = searchCombo->pathFromEnum(type);
    QCOMPARE(searchCombo->searchPath(), path);

}

QTEST_MAIN(AkonadiSearchDebugWidgetTest)
