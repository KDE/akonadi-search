/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "pimcontactsrunnerconfig.h"
using namespace Qt::Literals::StringLiterals;

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QCheckBox>
#include <QVBoxLayout>

K_PLUGIN_FACTORY(PIMContactsRunnerConfigFactory, registerPlugin<PIMContactsRunnerConfig>();)
PIMContactsRunnerConfig::PIMContactsRunnerConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , mQueryCompletionCheckBox(new QCheckBox(i18nc("@option:check", "Search in contacts indexed from sent and received emails too"), widget()))
{
    auto vbox = new QVBoxLayout(widget());

    connect(mQueryCompletionCheckBox, &QCheckBox::checkStateChanged, this, &PIMContactsRunnerConfig::configChanged);
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

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(u"krunnerrc"_s);
    KConfigGroup grp = cfg->group(u"Runners"_s);
    grp = KConfigGroup(&grp, u"PIM Contacts Search Runner"_s);

    mQueryCompletionCheckBox->setChecked(grp.readEntry(u"queryAutocompleter"_s, true));

    setNeedsSave(false);
}

void PIMContactsRunnerConfig::save()
{
    KCModule::save();

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(u"krunnerrc"_s);
    KConfigGroup grp = cfg->group(u"Runners"_s);
    grp = KConfigGroup(&grp, u"PIM Contacts Search Runner"_s);

    grp.writeEntry(u"queryAutocompleter"_s, mQueryCompletionCheckBox->isChecked());

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

#include "moc_pimcontactsrunnerconfig.cpp"
