/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef PIMCONTACTSRUNNER_H
#define PIMCONTACTSRUNNER_H

#include <KRunner/AbstractRunner>

class PIMContactsRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    explicit PIMContactsRunner(QObject *parent, const QVariantList &args);
    ~PIMContactsRunner() override;

    void reloadConfiguration() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
    QStringList categories() const override;
    QIcon categoryIcon(const QString &category) const override;

private:
    void queryContacts(Plasma::RunnerContext &context, const QString &queryString);
    void queryAutocompleter(Plasma::RunnerContext &context, const QString &queryString);

private:
    bool mQueryAutocompleter;
};

#endif // PIMCONTACTSRUNNER_H
