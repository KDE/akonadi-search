/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef PIMCONTACTSRUNNERCONFIG_H
#define PIMCONTACTSRUNNERCONFIG_H

#include <KCModule>

class QCheckBox;

class PIMContactsRunnerConfig : public KCModule
{
    Q_OBJECT
public:
    explicit PIMContactsRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    void configChanged();
    QCheckBox *mQueryCompletionCheckBox = nullptr;
};

#endif
