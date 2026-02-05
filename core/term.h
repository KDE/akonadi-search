/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include "search_core_export.h"

#include <QString>
#include <QVariant>

#include <memory>
class QDebug;
namespace Akonadi
{
namespace Search
{
class TermPrivate;

/*!
 * \class Akonadi::Search::Term
 * \inheader AkonadiSearch/Core/Term
 * \inmodule AkonadiSearch
 * \brief Search term for building complex search queries.
 *
 * The Term class represents a search condition or a combination of search conditions.
 * Terms can be simple property-value comparisons or complex nested structures combining
 * multiple terms with logical operations (AND/OR).
 *
 * Terms can be combined using operators: && for AND, || for OR, and ! for negation.
 *
 * \sa Query, Comparator, Operation
 */
class AKONADI_SEARCH_CORE_EXPORT Term
{
public:
    /*!
     * \enum Term::Comparator
     * \brief Defines comparison operations for term values.
     */
    enum Comparator : uint8_t {
        Auto, /*!< Automatically choose comparator based on value type. */
        Equal, /*!< Value must be equal. */
        Contains, /*!< Value must be contained (substring match). */
        Greater, /*!< Value must be greater than. */
        GreaterEqual, /*!< Value must be greater than or equal to. */
        Less, /*!< Value must be less than. */
        LessEqual /*!< Value must be less than or equal to. */
    };

    /*!
     * \enum Term::Operation
     * \brief Defines logical operations for combining terms.
     */
    enum Operation : uint8_t {
        None, /*!< No operation (single term). */
        And, /*!< Logical AND operation. */
        Or /*!< Logical OR operation. */
    };

    /*!
     * \brief Constructs an empty term.
     */
    Term();
    /*!
     * \brief Constructs a copy of the term \a t.
     * \param t The term to copy.
     */
    Term(const Term &t);

    /*!
     * \brief Constructs a term that requires a property to be present.
     * \param property The property name that must be present in search results.
     */
    Term(const QString &property);

    /*!
     * \brief Constructs a term that matches a property with a specific value.
     * \param property The property name.
     * \param value The value to match.
     * \param c The comparator to use. Defaults to Auto which:
     *          - Uses Contains for strings
     *          - Uses Contains for DateTime
     *          - Uses Equals for other types
     */
    Term(const QString &property, const QVariant &value, Comparator c = Auto);

    /*!
     * \brief Constructs a term combining other terms with the specified operation.
     * \param op The logical operation (AND or OR).
     */
    Term(Operation op);
    /*!
     * \brief Constructs a term combining a single sub-term with the specified operation.
     * \param op The logical operation (AND or OR).
     * \param t The sub-term.
     */
    Term(Operation op, const Term &t);
    /*!
     * \brief Constructs a term combining multiple sub-terms with the specified operation.
     * \param op The logical operation (AND or OR).
     * \param t The list of sub-terms.
     */
    Term(Operation op, const QList<Term> &t);
    /*!
     * \brief Constructs a term combining two terms with the specified operation.
     * \param lhs The left-hand side term.
     * \param op The logical operation (AND or OR).
     * \param rhs The right-hand side term.
     */
    Term(const Term &lhs, Operation op, const Term &rhs);
    /*!
     * \brief Destructs the term.
     */
    ~Term();

    /*!
     * \brief Checks if this term is valid.
     * \return \c true if the term is valid, \c false otherwise.
     */
    [[nodiscard]] bool isValid() const;

    /*!
     * \brief Sets the negation state of this term.
     * \param isNegated \c true to negate the term, \c false otherwise.
     *
     * Negation only applies for Equal or Contains comparators.
     * For other comparators, you must invert the logic manually.
     * \sa isNegated(), negated()
     */
    void setNegation(bool isNegated);

    /*!
     * \brief Checks if this term is negated.
     * \return \c true if the term is negated, \c false otherwise.
     * \sa setNegation()
     */
    [[nodiscard]] bool negated() const;
    /*!
     * \brief Checks if this term is negated (alias for negated()).
     * \return \c true if the term is negated, \c false otherwise.
     * \sa negated()
     */
    [[nodiscard]] bool isNegated() const;

    /*!
     * \brief Adds a sub-term to this term.
     * \param term The sub-term to add.
     * \sa setSubTerms(), subTerms()
     */
    void addSubTerm(const Term &term);
    /*!
     * \brief Sets the sub-terms for this term, replacing any existing sub-terms.
     * \param terms The list of sub-terms.
     * \sa addSubTerm(), subTerms()
     */
    void setSubTerms(const QList<Term> &terms);

    /*!
     * \brief Returns the first sub-term in the list of sub-terms.
     * \return The first sub-term, or an empty term if no sub-terms exist.
     * \sa subTerms()
     */
    [[nodiscard]] Term subTerm() const;
    /*!
     * \brief Returns all sub-terms for this term.
     * \return The list of sub-terms.
     * \sa addSubTerm(), setSubTerms()
     */
    [[nodiscard]] QList<Term> subTerms() const;

    /*!
     * \brief Sets the logical operation for combining sub-terms.
     * \param op The operation (AND or OR).
     * \sa operation()
     */
    void setOperation(Operation op);
    /*!
     * \brief Returns the logical operation for combining sub-terms.
     * \return The current operation.
     * \sa setOperation()
     */
    [[nodiscard]] Operation operation() const;

    /*!
     * \brief Checks if this term is empty.
     * \return \c true if the term is empty, \c false otherwise.
     * \sa empty()
     */
    [[nodiscard]] bool isEmpty() const;
    /*!
     * \brief Checks if this term is empty (alias for isEmpty()).
     * \return \c true if the term is empty, \c false otherwise.
     * \sa isEmpty()
     */
    [[nodiscard]] bool empty() const;

    /*!
     * \brief Returns the property targeted by this term.
     * \return The property name.
     * \sa setProperty()
     */
    [[nodiscard]] QString property() const;
    /*!
     * \brief Sets the property targeted by this term.
     * \param property The property name.
     * \sa property()
     */
    void setProperty(const QString &property);

    /*!
     * \brief Returns the value for this term.
     * \return The term value.
     * \sa setValue()
     */
    [[nodiscard]] QVariant value() const;
    /*!
     * \brief Sets the value for this term.
     * \param value The value to set.
     * \sa value()
     */
    void setValue(const QVariant &value);

    /*!
     * \brief Returns the comparator used by this term.
     * \return The current comparator.
     * \sa setComparator()
     */
    [[nodiscard]] Comparator comparator() const;
    /*!
     * \brief Sets the comparator for this term.
     * \param c The comparator to use.
     * \sa comparator()
     */
    void setComparator(Comparator c);

    /*!
     * \brief Sets custom user data for this term.
     * \param name The name of the user data.
     * \param value The value of the user data.
     * \sa userData()
     */
    void setUserData(const QString &name, const QVariant &value);
    /*!
     * \brief Returns custom user data for this term.
     * \param name The name of the user data.
     * \return The value of the user data, or an empty QVariant if not found.
     * \sa setUserData()
     */
    [[nodiscard]] QVariant userData(const QString &name) const;

    /*!
     * \brief Converts this term to a variant map representation.
     * \return A variant map representing the term.
     * \sa fromVariantMap()
     */
    [[nodiscard]] QVariantMap toVariantMap() const;
    /*!
     * \brief Creates a term from a variant map representation.
     * \param map The variant map representing a term.
     * \return A term reconstructed from the map data.
     * \sa toVariantMap()
     */
    static Term fromVariantMap(const QVariantMap &map);

    /*!
     * \brief Compares this term with another term for equality.
     * \param rhs The term to compare with.
     * \return \c true if the terms are equal, \c false otherwise.
     */
    bool operator==(const Term &rhs) const;

    /*!
     * \brief Assigns the contents of \a rhs to this term.
     * \param rhs The term to copy from.
     * \return A reference to this term.
     */
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

/*!
 * \brief Outputs a debug representation of a term.
 * \param d The debug stream.
 * \param t The term to output.
 * \return The debug stream.
 */
AKONADI_SEARCH_CORE_EXPORT QDebug operator<<(QDebug d, const Akonadi::Search::Term &t);
