/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once
#include "kcmutils_version.h"
#include <KCModule>

class QCheckBox;

class PIMContactsRunnerConfig : public KCModule
{
    Q_OBJECT
public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit PIMContactsRunnerConfig(QWidget *parent, const QVariantList &args);
#else
    explicit PIMContactsRunnerConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args = QVariantList());
#endif

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    void configChanged();
    QCheckBox *const mQueryCompletionCheckBox;
};
