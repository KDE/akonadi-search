/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef PIMCONTACTSRUNNER_H
#define PIMCONTACTSRUNNER_H

#include <KRunner/AbstractRunner>
#include <krunner_version.h>

class PIMContactsRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
#if KRUNNER_VERSION >= QT_VERSION_CHECK(5, 77, 0)
    explicit PIMContactsRunner(QObject *parent, const KPluginMetaData& metaData, const QVariantList &args);
#else
    explicit PIMContactsRunner(QObject *parent, const QVariantList &args);
#endif
    ~PIMContactsRunner() override;

    void reloadConfiguration() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

private:
    void queryContacts(Plasma::RunnerContext &context, const QString &queryString);
    void queryAutocompleter(Plasma::RunnerContext &context, const QString &queryString);

private:
    bool mQueryAutocompleter = true;
};

#endif // PIMCONTACTSRUNNER_H
