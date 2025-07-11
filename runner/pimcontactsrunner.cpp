/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "pimcontactsrunner.h"

#include "akonadi_runner_debug.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QDesktopServices>
#include <QIcon>
#include <QSharedPointer>
#include <QThread>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>

#include <KContacts/Addressee>

#include <KSharedConfig>

#include <KEmailAddress>

#include "lib/contactcompleter.h"
#include "lib/contactquery.h"
#include "lib/resultiterator.h"

#include <array>

Q_DECLARE_METATYPE(KContacts::Addressee *)

using namespace Qt::Literals::StringLiterals;
PIMContactsRunner::PIMContactsRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
    setObjectName("PIMContactsRunner"_L1);
    // reloadConfiguration() called by default init() implementation
}

PIMContactsRunner::~PIMContactsRunner() = default;

void PIMContactsRunner::reloadConfiguration()
{
    mQueryAutocompleter = config().readEntry(u"queryAutocompleter"_s, true);
}

void PIMContactsRunner::match(RunnerContext &context)
{
    const QString queryString = context.query();
    if (queryString.size() < 3) {
        return;
    }
    mListEmails.clear();

    queryContacts(context, queryString);

    qCDebug(AKONADI_KRUNNER_LOG) << this << "MATCH: queryAutocompleter =" << mQueryAutocompleter;
    if (mQueryAutocompleter) {
        queryAutocompleter(context, queryString);
    }
}

void PIMContactsRunner::queryContacts(RunnerContext &context, const QString &queryString)
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
    // a separate Session for each, otherwise things might explode
    QScopedPointer<Akonadi::Session, QScopedPointerDeleteLater> session(
        new Akonadi::Session("PIIMContactRunner-" + QByteArray::number((qlonglong)QThread::currentThread())));
    auto fetch = new Akonadi::ItemFetchJob(results, session.data());
    Akonadi::ItemFetchScope &scope = fetch->fetchScope();
    scope.fetchFullPayload(true);
    scope.setFetchRemoteIdentification(false);
    scope.setAncestorRetrieval(Akonadi::ItemFetchScope::None);

    if (!fetch->exec()) {
        qCWarning(AKONADI_KRUNNER_LOG) << "Error while fetching contacts:" << fetch->errorString();
        return;
    }

    const auto items = fetch->items();
    for (const Akonadi::Item &item : items) {
        KContacts::Addressee contact;
        try {
            contact = item.payload<KContacts::Addressee>();
        } catch (const Akonadi::Exception &e) {
            qCDebug(AKONADI_KRUNNER_LOG) << "Corrupted index? Index referrers to an Item without contact";
            // Error?
            continue;
        }

        if (contact.isEmpty()) {
            qCDebug(AKONADI_KRUNNER_LOG) << "Corrupted index? Index refers to an Item with an empty contact";
            continue;
        }

        const QStringList emails = contact.emails();
        if (emails.isEmpty()) {
            // No email, don't show the contact
            qCDebug(AKONADI_KRUNNER_LOG) << "Skipping" << contact.uid() << ", because it has no emails";
            continue;
        }

        QueryMatch match(this);
        match.setMatchCategory(i18n("Contacts"));
        match.setRelevance(0.75); // 0.75 is used by most runners, we don't

        const KContacts::Picture photo = contact.photo();
        if (!photo.isEmpty()) {
            const QImage img = photo.data().scaled(16, 16, Qt::KeepAspectRatio);
            match.setIcon(QIcon(QPixmap::fromImage(img)));
        } else {
            // The icon should be cached by Qt or FrameworkIntegration
            match.setIcon(QIcon::fromTheme(u"user-identity"_s));
        }

        QString matchedEmail;

        QString name = contact.formattedName();
        if (name.isEmpty()) {
            name = contact.assembledName();
        }

        // We got perfect match by name
        if (name == queryString) {
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);

            // We got perfect match by one of the email addresses
        } else if (emails.contains(queryString)) {
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
            matchedEmail = queryString;

            // We got partial match either by name, or email
        } else {
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Low);

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
            if (!mListEmails.contains(matchedEmail)) {
                mListEmails.append(matchedEmail);
                match.setText(i18nc("Name (email)", "%1 (%2)", name, matchedEmail));
                match.setData(u"mailto:%1<%2>"_s.arg(name, matchedEmail));
                context.addMatch(match);
            }
        } else {
            for (const QString &email : emails) {
                if (!mListEmails.contains(email)) {
                    mListEmails.append(email);
                    QueryMatch alternativeMatch = match;
                    alternativeMatch.setText(i18nc("Name (email)", "%1 (%2)", name, email));
                    alternativeMatch.setData(u"mailto:%1<%2>"_s.arg(name, email));
                    context.addMatch(alternativeMatch);
                }
            }
        }
    }
}

void PIMContactsRunner::queryAutocompleter(RunnerContext &context, const QString &queryString)
{
    Akonadi::Search::PIM::ContactCompleter completer(queryString);
    const QStringList completerResults = completer.complete();
    qCDebug(AKONADI_KRUNNER_LOG) << "Autocompleter returned" << completerResults.count() << "results";
    for (const QString &result : completerResults) {
        // Filter out results where writing a mail wouldn't make sense,
        // e.g. anything with noreply or various automatic emails from git forges
        static const std::array filters = {
            QRegularExpression(u"no[-]?reply@"_s),
            QRegularExpression(u"incoming\\+.+@invent\\.kde\\.org"_s),
            QRegularExpression(u"reply\\+.+@reply\\.github\\.com"_s),
            QRegularExpression(u"@noreply\\.github\\.com"_s),
            QRegularExpression(u"notifications@github\\.com"_s),
            QRegularExpression(u"incoming\\+.+@gitlab\\.com"_s),
            QRegularExpression(u"gitlab@gitlab\\.freedesktop\\.org"_s),
        };

        const bool skip = std::any_of(filters.cbegin(), filters.cend(), [result](const QRegularExpression &filter) {
            return result.contains(filter);
        });

        if (skip) {
            continue;
        }

        QueryMatch match(this);
        match.setRelevance(0.7); // slightly lower relevance than real addressbook contacts
        match.setMatchCategory(i18n("Contacts"));
        match.setSubtext(i18n("Autocompleted from received and sent emails"));
        match.setIcon(QIcon::fromTheme(u"user-identity"_s));
        if (result == queryString) {
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        } else {
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Low);
        }

        QString name;
        QString email;
        if (KEmailAddress::extractEmailAddressAndName(result, email, name)) {
            if (mListEmails.contains(email)) {
                continue;
            }
            mListEmails.append(email);
            if (name.isEmpty()) {
                match.setText(email);
                match.setData(u"mailto:%1"_s.arg(email));
            } else {
                match.setText(i18nc("Name (email)", "%1 (%2)", name, email));
                match.setData(u"mailto:%1<%2>"_s.arg(name, email));
            }
        } else {
            if (mListEmails.contains(result)) {
                continue;
            }
            mListEmails.append(result);
            match.setText(result);
            match.setData(u"mailto:%1"_s.arg(result));
        }
        context.addMatch(match);
    }
}

void PIMContactsRunner::run(const RunnerContext &context, const QueryMatch &match)
{
    Q_UNUSED(context)

    const QString mailto = match.data().toString();
    if (!mailto.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromUserInput(mailto));
    }
}

K_PLUGIN_CLASS_WITH_JSON(PIMContactsRunner, "plasma-krunner-pimcontacts.json")

#include "pimcontactsrunner.moc"

#include "moc_pimcontactsrunner.cpp"
