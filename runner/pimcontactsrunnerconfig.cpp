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

#include "pimcontactsrunnerconfig.h"
#include <kconfigwidgets_version.h>
#include <QCheckBox>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KPluginFactory>

K_PLUGIN_FACTORY(PIMContactsRunnerConfigFactory,
                 registerPlugin<PIMContactsRunnerConfig>(QStringLiteral("kcm_krunner_pimcontacts"));)

PIMContactsRunnerConfig::PIMContactsRunnerConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    mQueryCompletionCheckBox = new QCheckBox(i18n("Search in contacts indexed from sent and received emails too"));
    connect(mQueryCompletionCheckBox, &QCheckBox::stateChanged,
            this, &PIMContactsRunnerConfig::configChanged);

    vbox->addWidget(mQueryCompletionCheckBox);

    load();
}

void PIMContactsRunnerConfig::configChanged()
{
    Q_EMIT markAsChanged();
}

void PIMContactsRunnerConfig::load()
{
    KCModule::load();

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group(QStringLiteral("Runners"));
    grp = KConfigGroup(&grp, QStringLiteral("PIM Contacts Search Runner"));

    mQueryCompletionCheckBox->setChecked(grp.readEntry(QStringLiteral("queryAutocompleter"), true));

    Q_EMIT changed(false);
}

void PIMContactsRunnerConfig::save()
{
    KCModule::save();

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group(QStringLiteral("Runners"));
    grp = KConfigGroup(&grp, QStringLiteral("PIM Contacts Search Runner"));

    grp.writeEntry(QStringLiteral("queryAutocompleter"), mQueryCompletionCheckBox->isChecked());

    cfg->sync();

    Q_EMIT changed(false);
}

void PIMContactsRunnerConfig::defaults()
{
    KCModule::defaults();

    mQueryCompletionCheckBox->setChecked(true);

    Q_EMIT changed(true);
}

#include "pimcontactsrunnerconfig.moc"
