/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugsearchpathcomboboxtest.h"
#include "../akonadisearchdebugsearchpathcombobox.h"
#include <QTest>

AkonadiSearchDebugSearchPathComboBoxTest::AkonadiSearchDebugSearchPathComboBoxTest(QObject *parent)
    : QObject(parent)
{
}

AkonadiSearchDebugSearchPathComboBoxTest::~AkonadiSearchDebugSearchPathComboBoxTest() = default;

void AkonadiSearchDebugSearchPathComboBoxTest::shouldHaveDefaultValue()
{
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox combox;
    QVERIFY(combox.count() > 0);
}

void AkonadiSearchDebugSearchPathComboBoxTest::shouldReturnPath()
{
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox combox;
    QVERIFY(!combox.searchPath().isEmpty());
}

void AkonadiSearchDebugSearchPathComboBoxTest::shouldReturnCorrectSearchPath()
{
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox combox;
    QString path = combox.pathFromEnum(Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::Contacts);
    QCOMPARE(combox.searchPath(), path);
}

void AkonadiSearchDebugSearchPathComboBoxTest::shouldSelectCorrectType()
{
    Akonadi::Search::AkonadiSearchDebugSearchPathComboBox combox;
    QString path = combox.pathFromEnum(Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::ContactCompleter);
    combox.setSearchType(Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::ContactCompleter);
    QCOMPARE(combox.searchPath(), path);
    path = combox.pathFromEnum(Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::Emails);
    combox.setSearchType(Akonadi::Search::AkonadiSearchDebugSearchPathComboBox::Emails);
    QCOMPARE(combox.searchPath(), path);
}

QTEST_MAIN(AkonadiSearchDebugSearchPathComboBoxTest)
