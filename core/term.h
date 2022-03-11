/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_core_export.h"

#include <QDebug>
#include <QString>
#include <QVariant>

#include <memory>

namespace Akonadi
{
namespace Search
{
class TermPrivate;

/** Search term. */
class AKONADI_SEARCH_CORE_EXPORT Term
{
public:
    enum Comparator { Auto, Equal, Contains, Greater, GreaterEqual, Less, LessEqual };

    enum Operation { None, And, Or };

    Term();
    Term(const Term &t);

    /**
     * The Item must contain the property \p property
     */
    Term(const QString &property);

    /**
     * The Item must contain the property \p property with
     * value \value.
     *
     * The default comparator is Auto which has the following behavior
     * For Strings - Contains
     * For DateTime - Contains
     * For any other type - Equals
     */
    Term(const QString &property, const QVariant &value, Comparator c = Auto);

    /**
     * This term is a combination of other terms
     */
    Term(Operation op);
    Term(Operation op, const Term &t);
    Term(Operation op, const QList<Term> &t);
    Term(const Term &lhs, Operation op, const Term &rhs);
    ~Term();

    Q_REQUIRED_RESULT bool isValid() const;

    /**
     * Negate this term. Negation only applies for Equal or Contains
     * For other Comparators you must invert it yourself
     */
    void setNegation(bool isNegated);

    Q_REQUIRED_RESULT bool negated() const;
    Q_REQUIRED_RESULT bool isNegated() const;

    void addSubTerm(const Term &term);
    void setSubTerms(const QList<Term> &terms);

    /**
     * Returns the first subTerm in the list of subTerms
     */
    Q_REQUIRED_RESULT Term subTerm() const;
    Q_REQUIRED_RESULT QList<Term> subTerms() const;

    void setOperation(Operation op);
    Q_REQUIRED_RESULT Operation operation() const;

    Q_REQUIRED_RESULT bool isEmpty() const;
    Q_REQUIRED_RESULT bool empty() const;

    /**
     * Return the property this term is targeting
     */
    Q_REQUIRED_RESULT QString property() const;
    void setProperty(const QString &property);

    Q_REQUIRED_RESULT QVariant value() const;
    void setValue(const QVariant &value);

    Q_REQUIRED_RESULT Comparator comparator() const;
    void setComparator(Comparator c);

    void setUserData(const QString &name, const QVariant &value);
    Q_REQUIRED_RESULT QVariant userData(const QString &name) const;

    Q_REQUIRED_RESULT QVariantMap toVariantMap() const;
    static Term fromVariantMap(const QVariantMap &map);

    bool operator==(const Term &rhs) const;

    Term &operator=(const Term &rhs);

private:
    std::unique_ptr<TermPrivate> const d;
};

inline Term operator&&(const Term &lhs, const Term &rhs)
{
    Term t(Term::And);
    t.addSubTerm(lhs);
    t.addSubTerm(rhs);
    return t;
}

inline Term operator||(const Term &lhs, const Term &rhs)
{
    Term t(Term::Or);
    t.addSubTerm(lhs);
    t.addSubTerm(rhs);
    return t;
}

inline Term operator!(const Term &rhs)
{
    Term t(rhs);
    t.setNegation(!rhs.isNegated());
    return t;
}
}
}

AKONADI_SEARCH_CORE_EXPORT QDebug operator<<(QDebug d, const Akonadi::Search::Term &t);

