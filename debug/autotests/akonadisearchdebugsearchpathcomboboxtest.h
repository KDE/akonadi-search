/*
  SPDX-FileCopyrightText: 2014-2020 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef AKONADISEARCHDEBUGSEARCHPATHCOMBOBOXTEST_H
#define AKONADISEARCHDEBUGSEARCHPATHCOMBOBOXTEST_H

#include <QObject>

class AkonadiSearchDebugSearchPathComboBoxTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugSearchPathComboBoxTest(QObject *parent = nullptr);
    ~AkonadiSearchDebugSearchPathComboBoxTest();
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldReturnPath();
    void shouldReturnCorrectSearchPath();
    void shouldSelectCorrectType();
};

#endif // AKONADISEARCHDEBUGSEARCHPATHCOMBOBOXTEST_H
