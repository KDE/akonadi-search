/*
  SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKONADISEARCHDEBUGWIDGETTEST_H
#define AKONADISEARCHDEBUGWIDGETTEST_H

#include <QObject>

class AkonadiSearchDebugWidgetTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugWidgetTest(QObject *parent = nullptr);
    ~AkonadiSearchDebugWidgetTest();
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldFillLineEditWhenWeWantToSearchItem();
    void shouldEnabledPushButtonWhenLineEditIsNotEmpty();
    void shouldChangeSearchType();
};

#endif // AKONADISEARCHDEBUGWIDGETTEST_H
