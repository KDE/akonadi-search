/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

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

