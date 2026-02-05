/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "query.h"
#include "search_pim_export.h"

#include <Akonadi/Collection>
#include <QStringList>

#include <memory>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
class EmailQueryPrivate;

/*!
 * \class Akonadi::Search::PIM::EmailQuery
 * \inheader AkonadiSearch/PIM/EmailQuery
 * \inmodule AkonadiSearchPIM
 * \brief Search query for emails.
 *
 * EmailQuery allows searching for emails by various criteria including
 * sender, recipient, subject, body, collections, attachment status,
 * and importance/read flags.
 *
 * \sa Query, ResultIterator
 */
class AKONADI_SEARCH_PIM_EXPORT EmailQuery : public Query
{
public:
    /*!
     * \enum EmailQuery::OpType
     * \brief Defines logical operations for combining email search criteria.
     */
    enum OpType : uint8_t {
        OpAnd = 0, /*!< AND operation. */
        OpOr, /*!< OR operation. */
    };

    /*!
     * \brief Constructs an empty email query.
     */
    EmailQuery();
    /*!
     * \brief Destructs the email query.
     */
    ~EmailQuery() override;

    /*!
     * \brief Sets whether to split the search match string.
     * \param split \c true to split the search string, \c false to keep it as-is.
     */
    void setSplitSearchMatchString(bool split);

    /*!
     * \brief Sets the logical operation for combining search criteria.
     * \param op The operation type (AND or OR).
     */
    void setSearchType(OpType op);

    /*!
     * \brief Sets the list of email addresses involved in the emails.
     * \param involves The list of email addresses.
     * \sa addInvolves()
     */
    void setInvolves(const QStringList &involves);
    /*!
     * \brief Adds an email address to the involved addresses filter.
     * \param email The email address to add.
     * \sa setInvolves()
     */
    void addInvolves(const QString &email);

    /*!
     * \brief Sets the list of recipient addresses.
     * \param to The list of "To" addresses.
     * \sa addTo()
     */
    void setTo(const QStringList &to);
    /*!
     * \brief Adds a recipient address.
     * \param to The "To" address to add.
     * \sa setTo()
     */
    void addTo(const QString &to);

    /*!
     * \brief Sets the sender address.
     * \param from The sender email address.
     * \sa addFrom()
     */
    void setFrom(const QString &from);
    /*!
     * \brief Adds a sender address.
     * \param from The sender email address to add.
     * \sa setFrom()
     */
    void addFrom(const QString &from);

    /*!
     * \brief Sets the list of CC addresses.
     * \param cc The list of CC addresses.
     * \sa addCc()
     */
    void setCc(const QStringList &cc);
    /*!
     * \brief Adds a CC address.
     * \param cc The CC address to add.
     * \sa setCc()
     */
    void addCc(const QString &cc);

    /*!
     * \brief Sets the list of BCC addresses.
     * \param bcc The list of BCC addresses.
     * \sa addBcc()
     */
    void setBcc(const QStringList &bcc);
    /*!
     * \brief Adds a BCC address.
     * \param bcc The BCC address to add.
     * \sa setBcc()
     */
    void addBcc(const QString &bcc);

    /*!
     * \brief Sets the collections to search in.
     * \param collections The list of collection IDs.
     * \sa addCollection()
     */
    void setCollection(const QList<Akonadi::Collection::Id> &collections);
    /*!
     * \brief Adds a collection to the search.
     * \param id The collection ID to add.
     * \sa setCollection()
     */
    void addCollection(Akonadi::Collection::Id id);

    /*!
     * \brief Filters for important emails.
     * \param important \c true to search for important emails only. By default ignored.
     */
    void setImportant(bool important = true);

    /*!
     * \brief Filters for read/unread emails.
     * \param read \c true to search for read emails only. By default ignored.
     */
    void setRead(bool read = true);

    /*!
     * \brief Filters for emails with attachments.
     * \param hasAttachment \c true to search for emails with attachments. By default ignored.
     */
    void setAttachment(bool hasAttachment = true);

    /*!
     * \brief Matches the string anywhere in the entire email.
     * \param match The string to match in the email body and subject.
     */
    void matches(const QString &match);

    /*!
     * \brief Matches a string specifically in the email subject.
     * \param subjectMatch The string to match in the subject.
     */
    void subjectMatches(const QString &subjectMatch);

    /*!
     * \brief Matches a string specifically in the email body.
     * \param bodyMatch The string to match in the body.
     */
    void bodyMatches(const QString &bodyMatch);

    /*!
     * \brief Sets the maximum number of results to return.
     * \param limit The result limit.
     */
    void setLimit(int limit);
    /*!
     * \brief Returns the maximum number of results for this query.
     * \return The result limit.
     */
    [[nodiscard]] int limit() const;

    /*!
     * \brief Executes the query and returns an iterator to fetch results.
     * \return An iterator over the search results.
     */
    [[nodiscard]] ResultIterator exec() override;

private:
    std::unique_ptr<EmailQueryPrivate> const d;
};
}
}
}
