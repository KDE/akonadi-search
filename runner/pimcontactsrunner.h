/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <KRunner/AbstractRunner>

class PIMContactsRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    explicit PIMContactsRunner(QObject *parent, const KPluginMetaData &metaData);
    ~PIMContactsRunner() override;

    void reloadConfiguration() override;

    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

private:
    void queryContacts(KRunner::RunnerContext &context, const QString &queryString);
    void queryAutocompleter(KRunner::RunnerContext &context, const QString &queryString);

private:
    bool mQueryAutocompleter = true;
    QStringList mListEmails;
};
