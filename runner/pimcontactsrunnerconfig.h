/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once
#include <KCModule>

class QCheckBox;

class PIMContactsRunnerConfig : public KCModule
{
    Q_OBJECT
public:
    explicit PIMContactsRunnerConfig(QObject *parent, const KPluginMetaData &data);

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    void configChanged();
    QCheckBox *const mQueryCompletionCheckBox;
};
