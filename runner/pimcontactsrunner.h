/*
 * Copyright (C) 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    virtual ~PIMContactsRunner();

    void reloadConfiguration() Q_DECL_OVERRIDE;

    void match(Plasma::RunnerContext &context) Q_DECL_OVERRIDE;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) Q_DECL_OVERRIDE;
    QStringList categories() const Q_DECL_OVERRIDE;
    QIcon categoryIcon(const QString &category) const Q_DECL_OVERRIDE;

private:
    void queryContacts(Plasma::RunnerContext &context, const QString &queryString);
    void queryAutocompleter(Plasma::RunnerContext &context, const QString &queryString);

private:
    bool mQueryAutocompleter;
};

#endif // PIMCONTACTSRUNNER_H
