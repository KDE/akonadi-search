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
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
PIMContactsRunnerConfig::PIMContactsRunnerConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , mQueryCompletionCheckBox(new QCheckBox(i18n("Search in contacts indexed from sent and received emails too"), this))
#else
PIMContactsRunnerConfig::PIMContactsRunnerConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , mQueryCompletionCheckBox(new QCheckBox(i18n("Search in contacts indexed from sent and received emails too"), widget()))
#endif
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto vbox = new QVBoxLayout(this);
#else
    auto vbox = new QVBoxLayout(widget());
#endif

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

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    Q_EMIT changed(false);
#else
    setNeedsSave(false);
#endif
}

void PIMContactsRunnerConfig::save()
{
    KCModule::save();

    const KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup grp = cfg->group(QStringLiteral("Runners"));
    grp = KConfigGroup(&grp, QStringLiteral("PIM Contacts Search Runner"));

    grp.writeEntry(QStringLiteral("queryAutocompleter"), mQueryCompletionCheckBox->isChecked());

    cfg->sync();
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    Q_EMIT changed(false);
#else
    setNeedsSave(false);
#endif
}

void PIMContactsRunnerConfig::defaults()
{
    KCModule::defaults();

    mQueryCompletionCheckBox->setChecked(true);
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    Q_EMIT changed(true);
#else
    setNeedsSave(true);
#endif
}

#include "pimcontactsrunnerconfig.moc"
