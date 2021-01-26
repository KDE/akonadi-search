/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#ifndef AKONADI_SEARCH_PIM_EMAIL_QUERY_H
#define AKONADI_SEARCH_PIM_EMAIL_QUERY_H

#include "query.h"
#include "search_pim_export.h"

#include <Collection>
#include <QStringList>

namespace Akonadi
{
namespace Search
{
namespace PIM
{
/** Email query. */
class AKONADI_SEARCH_PIM_EXPORT EmailQuery : public Query
{
public:
    EmailQuery();
    ~EmailQuery() override;

    enum OpType { OpAnd = 0, OpOr };

    void setSplitSearchMatchString(bool split);

    void setSearchType(OpType op);

    void setInvolves(const QStringList &involves);
    void addInvolves(const QString &email);

    void setTo(const QStringList &to);
    void addTo(const QString &to);

    void setFrom(const QString &from);
    void addFrom(const QString &from);

    void setCc(const QStringList &cc);
    void addCc(const QString &cc);

    void setBcc(const QStringList &bcc);
    void addBcc(const QString &bcc);

    void setCollection(const QList<Akonadi::Collection::Id> &collections);
    void addCollection(Akonadi::Collection::Id id);

    /**
     * By default the importance is ignored
     */
    void setImportant(bool important = true);

    /**
     * By default the read status is ignored
     */
    void setRead(bool read = true);

    /**
     * By default the attachment status is ignored
     */
    void setAttachment(bool hasAttachment = true);

    /**
     * Matches the string \p match anywhere in the entire email
     * body
     */
    void matches(const QString &match);

    /**
     * Matches the string \p subjectMatch specifically in the
     * email subject
     */
    void subjectMatches(const QString &subjectMatch);

    /**
     * Matches the string \p bodyMatch specifically in the body email
     */
    void bodyMatches(const QString &bodyMatch);

    void setLimit(int limit);
    int limit() const;

    /**
     * Execute the query and return an iterator to fetch
     * the results
     */
    ResultIterator exec() override;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};
}
}
}

#endif // AKONADI_SEARCH_PIM_EMAIL_QUERY_H
