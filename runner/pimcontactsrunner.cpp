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

#include "pimcontactsrunner.h"
#include "akonadi_runner_debug.h"

#include <KLocalizedString>

#include <QIcon>
#include <QVector>
#include <QSharedPointer>
#include <QThread>
#include <QDesktopServices>

#include <AkonadiCore/Session>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <KContacts/Addressee>

#include <KSharedConfig>
#include <KConfigGroup>

#include <KEmailAddress>

#include "lib/contactquery.h"
#include "lib/contactcompleter.h"
#include "lib/resultiterator.h"

static const QStringList sCategories = { i18n("Contacts") };

Q_DECLARE_METATYPE(KContacts::Addressee *)

PIMContactsRunner::PIMContactsRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
    , mQueryAutocompleter(true)
{
    setObjectName(QStringLiteral("PIMContactsRunner"));
    setSpeed(Plasma::AbstractRunner::SlowSpeed);
    setIgnoredTypes(Plasma::RunnerContext::FileSystem
                    | Plasma::RunnerContext::Executable
                    | Plasma::RunnerContext::NetworkLocation);

    // reloadConfiguration() called by default init() implementation
}

PIMContactsRunner::~PIMContactsRunner()
{
}

void PIMContactsRunner::reloadConfiguration()
{
    mQueryAutocompleter = config().readEntry(QStringLiteral("queryAutocompleter"), true);
}

QStringList PIMContactsRunner::categories() const
{
    return sCategories;
}

QIcon PIMContactsRunner::categoryIcon(const QString &category) const
{
    if (category == sCategories[0]) {
        return QIcon::fromTheme(QStringLiteral("view-pim-contacts"));
    }

    return Plasma::AbstractRunner::categoryIcon(category);
}

void PIMContactsRunner::match(Plasma::RunnerContext &context)
{
    const QString queryString = context.query();
    if (queryString.size() < 3) {
        return;
    }

    queryContacts(context, queryString);

    qDebug() << this << "MATCH: queryAutocompleter =" << mQueryAutocompleter;
    if (mQueryAutocompleter) {
        queryAutocompleter(context, queryString);
    }
}

void PIMContactsRunner::queryContacts(Plasma::RunnerContext &context,
                                      const QString &queryString)
{
    Akonadi::Search::PIM::ContactQuery query;
    query.matchName(queryString);
    query.matchEmail(queryString);
    query.setMatchCriteria(Akonadi::Search::PIM::ContactQuery::StartsWithMatch);
    // Does not make sense to list more than 50 contacts on broad search terms
    query.setLimit(50);

    // Accumulate the results so that we can fetch all in single Akonadi request
    Akonadi::Search::PIM::ResultIterator iter = query.exec();
    QList<Akonadi::Item::Id> results;
    while (iter.next()) {
        results.push_back(iter.id());
    }

    qCDebug(AKONADI_KRUNNER_LOG) << "Query:" << queryString << ", results:" << results.count();

    if (results.isEmpty()) {
        return;
    }

    // There can be multiple queries running at the same time, make sure we have
    // a separete Session for each, otherwise things might explode
    QScopedPointer<Akonadi::Session, QScopedPointerDeleteLater> session(
        new Akonadi::Session("PIIMContactRunner-" + QByteArray::number((qlonglong)QThread::currentThread())));
    Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob(results, session.data());
    Akonadi::ItemFetchScope &scope = fetch->fetchScope();
    scope.fetchFullPayload(true);
    scope.setFetchRemoteIdentification(false);
    scope.setAncestorRetrieval(Akonadi::ItemFetchScope::None);

    if (!fetch->exec()) {
        qCWarning(AKONADI_KRUNNER_LOG) << "Error while fetching contacts:" << fetch->errorString();
        return;
    }

    for (const Akonadi::Item &item : fetch->items()) {

        KContacts::Addressee contact;
        try {
            contact = item.payload<KContacts::Addressee>();
        } catch (const Akonadi::Exception &e) {
            qCDebug(AKONADI_KRUNNER_LOG) << "Corrutped index? Index referrers to an Item without contact";
            // Error?
            continue;
        }

        if (contact.isEmpty()) {
            qCDebug(AKONADI_KRUNNER_LOG) << "Corrupted index? Index referrs to an Item with an empty contact";
            continue;
        }

        const QStringList emails = contact.emails();
        if (emails.isEmpty()) {
            // No email, don't show the contact
            qCDebug(AKONADI_KRUNNER_LOG) << "Skipping" << contact.uid() << ", because it has no emails";
            continue;
        }

        Plasma::QueryMatch match(this);
        match.setMatchCategory(sCategories[0]);
        match.setRelevance(0.75); // 0.75 is used by most runners, we don't
        // want to shadow them
        match.setMimeType(KContacts::Addressee::mimeType());

        const KContacts::Picture photo = contact.photo();
        if (!photo.isEmpty()) {
            const QImage img = photo.data().scaled(16, 16, Qt::KeepAspectRatio);
            match.setIcon(QIcon(QPixmap::fromImage(img)));
        } else {
            // The icon should be cached by Qt or FrameworkIntegration
            match.setIcon(QIcon::fromTheme(QStringLiteral("user-identity")));
        }

        QString matchedEmail;

        QString name = contact.formattedName();
        if (name.isEmpty()) {
            name = contact.assembledName();
        }

        // We got perfect match by name
        if (name == queryString) {
            match.setType(Plasma::QueryMatch::ExactMatch);

            // We got perfect match by one of the email addresses
        } else if (emails.contains(queryString)) {
            match.setType(Plasma::QueryMatch::ExactMatch);
            matchedEmail = queryString;

            // We got partial match either by name, or email
        } else {
            match.setType(Plasma::QueryMatch::PossibleMatch);

            // See if the match was by one of the email addresses
            for (const QString &email : emails) {
                if (email.startsWith(queryString)) {
                    matchedEmail = email;
                    break;
                }
            }
        }

        // If we had an email match, then use it, otherwise assume name-based
        // match and explode the contact to all available email addresses
        if (!matchedEmail.isEmpty()) {
            match.setText(i18nc("Name (email)", "%1 (%2)", name, matchedEmail));
            match.setData(QStringLiteral("mailto:%1<%2>").arg(name, matchedEmail));
            context.addMatch(match);
        } else {
            for (const QString &email : emails) {
                Plasma::QueryMatch alternativeMatch = match;
                alternativeMatch.setText(i18nc("Name (email)", "%1 (%2)", name, email));
                alternativeMatch.setData(QStringLiteral("mailto:%1<%2>").arg(name, email));
                context.addMatch(alternativeMatch);
            }
        }

    }
}

void PIMContactsRunner::queryAutocompleter(Plasma::RunnerContext &context,
        const QString &queryString)
{
    Akonadi::Search::PIM::ContactCompleter completer(queryString);
    const QStringList completerResults = completer.complete();
    qCDebug(AKONADI_KRUNNER_LOG) << "Autocompleter returned"  << completerResults.count() << "results";
    for (const QString &result : completerResults) {
        Plasma::QueryMatch match(this);
        match.setRelevance(0.7); // slightly lower relevance than real addressbook contacts
        match.setMimeType(KContacts::Addressee::mimeType());
        match.setMatchCategory(sCategories[0]);
        match.setSubtext(i18n("Autocompleted from received and sent emails"));
        match.setIcon(QIcon::fromTheme(QStringLiteral("user-identity")));
        if (result == queryString) {
            match.setType(Plasma::QueryMatch::ExactMatch);
        } else {
            match.setType(Plasma::QueryMatch::PossibleMatch);
        }

        QString name, email;
        if (KEmailAddress::extractEmailAddressAndName(result, email, name)) {
            if (name.isEmpty()) {
                match.setText(email);
                match.setData(QStringLiteral("mailto:%1").arg(email));
            } else {
                match.setText(i18nc("Name (email)", "%1 (%2)", name, email));
                match.setData(QStringLiteral("mailto:%1<%2>").arg(name, email));
            }
        } else {
            match.setText(result);
            match.setData(QStringLiteral("mailto:%1").arg(result));
        }
        context.addMatch(match);
    }
}

void PIMContactsRunner::run(const Plasma::RunnerContext &context,
                            const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const QString mailto = match.data().toString();
    if (!mailto.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromUserInput(mailto));
    }
}

K_EXPORT_PLASMA_RUNNER(pimcontactsrunner, PIMContactsRunner)

#include "pimcontactsrunner.moc"
