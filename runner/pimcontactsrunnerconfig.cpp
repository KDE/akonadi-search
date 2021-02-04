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

K_PLUGIN_FACTORY(PIMContactsRunnerConfigFactory, registerPlugin<PIMContactsRunnerConfig>(QStringLiteral("kcm_krunner_pimcontacts"));)

PIMContactsRunnerConfig::PIMContactsRunnerConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    auto vbox = new QVBoxLayout(this);

    mQueryCompletionCheckBox = new QCheckBox(i18n("Search in contacts indexed from sent and received emails too"), this);
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
