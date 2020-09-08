/*
  SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKONADISEARCHDEBUGDIALOGTEST_H
#define AKONADISEARCHDEBUGDIALOGTEST_H

#include <QObject>

class AkonadiSearchDebugDialogTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugDialogTest(QObject *parent = nullptr);
    ~AkonadiSearchDebugDialogTest();

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldFillLineEditWhenWeWantToSearchItem();

    void initTestCase();
};

#endif // AKONADISEARCHDEBUGDIALOGTEST_H
