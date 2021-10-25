/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class AkonadiSearchDebugSearchPathComboBoxTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugSearchPathComboBoxTest(QObject *parent = nullptr);
    ~AkonadiSearchDebugSearchPathComboBoxTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldReturnPath();
    void shouldReturnCorrectSearchPath();
    void shouldSelectCorrectType();
};

