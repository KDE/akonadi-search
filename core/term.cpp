/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#include "term.h"

#include <QDateTime>
#include <QDebug>
using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search;

class Akonadi::Search::TermPrivate
{
public:
    Term::Operation m_op = Term::None;
    Term::Comparator m_comp = Term::Auto;

    QString m_property;
    QVariant m_value;

    bool m_isNegated = false;

    QList<Term> m_subTerms;
    QVariantHash m_userData;
};

Term::Term()
    : d(new TermPrivate)
{
}

Term::Term(const Term &t)
    : d(new TermPrivate(*t.d))
{
}

Term::Term(const QString &property)
    : d(new TermPrivate)
{
    d->m_property = property;
}

Term::Term(const QString &property, const QVariant &value, Term::Comparator c)
    : d(new TermPrivate)
{
    d->m_property = property;
    d->m_value = value;

    if (c == Auto) {
        auto valueType = value.metaType().id();
        if (valueType == QMetaType::QString) {
            d->m_comp = Contains;
        } else if (valueType == QMetaType::QDateTime) {
            d->m_comp = Contains;
        } else {
            d->m_comp = Equal;
        }
    } else {
        d->m_comp = c;
    }
}

/*
Term::Term(const QString& property, const QVariant& start, const QVariant& end)
    : d(new TermPrivate)
{
    d->m_property = property;
    d->m_op = Range;

    // FIXME: How to save range queries?
}
*/

Term::Term(Term::Operation op)
    : d(new TermPrivate)
{
    d->m_op = op;
}

Term::Term(Term::Operation op, const Term &t)
    : d(new TermPrivate)
{
    d->m_op = op;
    d->m_subTerms << t;
}

Term::Term(Term::Operation op, const QList<Term> &t)
    : d(new TermPrivate)
{
    d->m_op = op;
    d->m_subTerms = t;
}

Term::Term(const Term &lhs, Term::Operation op, const Term &rhs)
    : d(new TermPrivate)
{
    d->m_op = op;
    d->m_subTerms << lhs;
    d->m_subTerms << rhs;
}

Term::~Term() = default;

bool Term::isValid() const
{
    if (d->m_property.isEmpty()) {
        if (d->m_op == Term::None) {
            return false;
        }

        return d->m_value.isNull();
    }

    return true;
}

void Term::setNegation(bool isNegated)
{
    d->m_isNegated = isNegated;
}

bool Term::isNegated() const
{
    return d->m_isNegated;
}

bool Term::negated() const
{
    return d->m_isNegated;
}

void Term::addSubTerm(const Term &term)
{
    d->m_subTerms << term;
}

void Term::setSubTerms(const QList<Term> &terms)
{
    d->m_subTerms = terms;
}

Term Term::subTerm() const
{
    if (!d->m_subTerms.isEmpty()) {
        return d->m_subTerms.first();
    }

    return {};
}

QList<Term> Term::subTerms() const
{
    return d->m_subTerms;
}

void Term::setOperation(Term::Operation op)
{
    d->m_op = op;
}

Term::Operation Term::operation() const
{
    return d->m_op;
}

bool Term::empty() const
{
    return isEmpty();
}

bool Term::isEmpty() const
{
    return d->m_property.isEmpty() && d->m_value.isNull() && d->m_subTerms.isEmpty();
}

QString Term::property() const
{
    return d->m_property;
}

void Term::setProperty(const QString &property)
{
    d->m_property = property;
}

void Term::setValue(const QVariant &value)
{
    d->m_value = value;
}

QVariant Term::value() const
{
    return d->m_value;
}

Term::Comparator Term::comparator() const
{
    return d->m_comp;
}

void Term::setComparator(Term::Comparator c)
{
    d->m_comp = c;
}

void Term::setUserData(const QString &name, const QVariant &value)
{
    d->m_userData.insert(name, value);
}

QVariant Term::userData(const QString &name) const
{
    return d->m_userData.value(name);
}

QVariantMap Term::toVariantMap() const
{
    QVariantMap map;
    if (d->m_op != None) {
        QVariantList variantList;
        variantList.reserve(d->m_subTerms.count());
        for (const Term &term : std::as_const(d->m_subTerms)) {
            variantList << QVariant(term.toVariantMap());
        }

        if (d->m_op == And) {
            map[u"$and"_s] = variantList;
        } else {
            map[u"$or"_s] = variantList;
        }

        return map;
    }

    QString op;
    switch (d->m_comp) {
    case Equal:
        map[d->m_property] = d->m_value;
        return map;

    case Contains:
        op = u"$ct"_s;
        break;

    case Greater:
        op = u"$gt"_s;
        break;

    case GreaterEqual:
        op = u"$gte"_s;
        break;

    case Less:
        op = u"$lt"_s;
        break;

    case LessEqual:
        op = u"$lte"_s;
        break;

    default:
        return {};
    }

    QVariantMap m;
    m[op] = d->m_value;
    map[d->m_property] = QVariant(m);

    return map;
}

namespace
{
// QJson does not recognize QDate/QDateTime parameters. We try to guess
// and see if they can be converted into date/datetime.
QVariant tryConvert(const QVariant &var)
{
    if (var.canConvert<QDateTime>()) {
        QDateTime dt = var.toDateTime();
        if (!dt.isValid()) {
            return var;
        }

        if (!var.toString().contains(u'T')) {
            return QVariant(var.toDate());
        }
        return dt;
    }
    return var;
}
}

Term Term::fromVariantMap(const QVariantMap &map)
{
    if (map.size() != 1) {
        return {};
    }

    Term term;

    QString andOrString;
    if (map.contains(u"$and"_s)) {
        andOrString = u"$and"_s;
        term.setOperation(And);
    } else if (map.contains(u"$or"_s)) {
        andOrString = u"$or"_s;
        term.setOperation(Or);
    }

    if (!andOrString.isEmpty()) {
        QList<Term> subTerms;

        const QVariantList list = map[andOrString].toList();
        subTerms.reserve(list.count());
        for (const QVariant &var : list) {
            subTerms << Term::fromVariantMap(var.toMap());
        }

        term.setSubTerms(subTerms);
        return term;
    }

    QString prop = map.cbegin().key();
    term.setProperty(prop);

    QVariant value = map.value(prop);
    if (value.userType() == QMetaType::QVariantMap) {
        QVariantMap map = value.toMap();
        if (map.size() != 1) {
            return term;
        }

        QString op = map.cbegin().key();
        Term::Comparator com;
        if (op == "$ct"_L1) {
            com = Contains;
        } else if (op == "$gt"_L1) {
            com = Greater;
        } else if (op == "$gte"_L1) {
            com = GreaterEqual;
        } else if (op == "$lt"_L1) {
            com = Less;
        } else if (op == "$lte"_L1) {
            com = LessEqual;
        } else {
            return term;
        }

        term.setComparator(com);
        term.setValue(tryConvert(map.value(op)));

        return term;
    }

    term.setComparator(Equal);
    term.setValue(tryConvert(value));

    return term;
}

bool Term::operator==(const Term &rhs) const
{
    if (d->m_op != rhs.d->m_op || d->m_comp != rhs.d->m_comp || d->m_isNegated != rhs.d->m_isNegated || d->m_property != rhs.d->m_property
        || d->m_value != rhs.d->m_value) {
        return false;
    }

    if (d->m_subTerms.size() != rhs.d->m_subTerms.size()) {
        return false;
    }

    if (d->m_subTerms.isEmpty()) {
        return true;
    }

    for (const Term &t : std::as_const(d->m_subTerms)) {
        if (!rhs.d->m_subTerms.contains(t)) {
            return false;
        }
    }

    return true;
}

Term &Term::operator=(const Term &rhs)
{
    *d = *rhs.d;
    return *this;
}

namespace
{
QString comparatorToString(Term::Comparator c)
{
    switch (c) {
    case Term::Auto:
        return u"Auto"_s;
    case Term::Equal:
        return u"="_s;
    case Term::Contains:
        return u":"_s;
    case Term::Less:
        return u"<"_s;
    case Term::LessEqual:
        return u"<="_s;
    case Term::Greater:
        return u">"_s;
    case Term::GreaterEqual:
        return u">="_s;
    }

    return {};
}

QString operationToString(Term::Operation op)
{
    switch (op) {
    case Term::None:
        return u"NONE"_s;
    case Term::And:
        return u"AND"_s;
    case Term::Or:
        return u"OR"_s;
    }

    return {};
}
} // namespace

QDebug operator<<(QDebug d, const Term &t)
{
    if (t.subTerms().isEmpty()) {
        d << u"(%1 %2 %3 (%4))"_s.arg(t.property(), comparatorToString(t.comparator()), t.value().toString(), QString::fromLatin1(t.value().typeName()))
                 .toUtf8()
                 .constData();
    } else {
        d << "(" << operationToString(t.operation()).toUtf8().constData();
        const QList<Term> subterms = t.subTerms();
        for (const Term &term : std::as_const(subterms)) {
            d << term;
        }
        d << ")";
    }
    return d;
}
