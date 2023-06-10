/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "pimcontactsrunnerconfig.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QCheckBox>
#include <QVBoxLayout>

K_PLUGIN_FACTORY(PIMContactsRunnerConfigFactory, registerPlugin<PIMContactsRunnerConfig>();)
PIMContactsRunnerConfig::PIMContactsRunnerConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , mQueryCompletionCheckBox(new QCheckBox(i18n("Search in contacts indexed from sent and received emails too"), widget()))
{
    auto vbox = new QVBoxLayout(widget());

    connect(mQueryCompletionCheckBox, &QCheckBox::stateChanged, this, &PIMContactsRunnerConfig::configChanged);

    vbox->addWidget(mQueryCompletionCheckBox);

    load();
}

void PIMContactsRunnerConfig::configChanged()
{
    markAsChanged();
}

void PIMContactsRunnerConfig::load()
{
    KCModule::load();

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group(QStringLiteral("Runners"));
    grp = KConfigGroup(&grp, QStringLiteral("PIM Contacts Search Runner"));

    mQueryCompletionCheckBox->setChecked(grp.readEntry(QStringLiteral("queryAutocompleter"), true));

    setNeedsSave(false);
}

void PIMContactsRunnerConfig::save()
{
    KCModule::save();

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group(QStringLiteral("Runners"));
    grp = KConfigGroup(&grp, QStringLiteral("PIM Contacts Search Runner"));

    grp.writeEntry(QStringLiteral("queryAutocompleter"), mQueryCompletionCheckBox->isChecked());

    cfg->sync();
    setNeedsSave(false);
}

void PIMContactsRunnerConfig::defaults()
{
    KCModule::defaults();

    mQueryCompletionCheckBox->setChecked(true);
    setNeedsSave(true);
}

#include "pimcontactsrunnerconfig.moc"
