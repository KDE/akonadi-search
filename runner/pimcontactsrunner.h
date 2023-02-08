/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <KRunner/AbstractRunner>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace Plasma;
class PIMContactsRunner : public Plasma::AbstractRunner
#else
using namespace KRunner;
class PIMContactsRunner : public KRunner::AbstractRunner
#endif
{
    Q_OBJECT

public:
    explicit PIMContactsRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~PIMContactsRunner() override;

    void reloadConfiguration() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;

private:
    void queryContacts(RunnerContext &context, const QString &queryString);
    void queryAutocompleter(RunnerContext &context, const QString &queryString);

private:
    bool mQueryAutocompleter = true;
};
