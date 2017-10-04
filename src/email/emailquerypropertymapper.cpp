/*
 * Copyright (C) 2017  Daniel Vrátil <dvratil@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "emailquerypropertymapper.h"

#include <AkonadiCore/SearchQuery>

#include <QMutex>

using namespace Akonadi::Search;

EmailQueryPropertyMapper *EmailQueryPropertyMapper::sInstance = nullptr;

EmailQueryPropertyMapper::EmailQueryPropertyMapper()
{
    insertPrefix(Akonadi::EmailSearchTerm::HeaderFrom, QStringLiteral("F"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderTo, QStringLiteral("T"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderCC, QStringLiteral("CC"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderBCC, QStringLiteral("BC"));
    insertPrefix(Akonadi::EmailSearchTerm::Subject, QStringLiteral("SU"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderReplyTo, QStringLiteral("RT"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderOrganization, QStringLiteral("O"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderListId, QStringLiteral("LI"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderResentFrom, QStringLiteral("RF"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderXLoop, QStringLiteral("XL"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderXMailingList, QStringLiteral("XML"));
    insertPrefix(Akonadi::EmailSearchTerm::HeaderXSpamFlag, QStringLiteral("XSF"));
    insertPrefix(Akonadi::EmailSearchTerm::Body, QStringLiteral("BO"));
    insertPrefix(Akonadi::EmailSearchTerm::Headers, QStringLiteral("HE"));

    // TODO: Add body flag?
    // TODO: Add tags?

    // Boolean Flags
    insertPrefix(IsImportantFlag, QStringLiteral("I"));
    insertPrefix(IsToActFlag, QStringLiteral("T"));
    insertPrefix(IsWatchedFlag, QStringLiteral("W"));
    insertPrefix(IsDeletedFlag, QStringLiteral("D"));
    insertPrefix(IsSpamFlag, QStringLiteral("S"));
    insertPrefix(IsRepliedFlag, QStringLiteral("E"));
    insertPrefix(IsIgnoredFlag, QStringLiteral("G"));
    insertPrefix(IsForwardedFlag, QStringLiteral("F"));
    insertPrefix(IsSentFlag, QStringLiteral("N"));
    insertPrefix(IsQueuedFlag, QStringLiteral("Q"));
    insertPrefix(IsHamFlag, QStringLiteral("H"));
    insertPrefix(IsReadFlag, QStringLiteral("R"));
    insertPrefix(HasAttachmentFlag, QStringLiteral("A"));
    insertPrefix(IsEncryptedFlag, QStringLiteral("C"));
    insertPrefix(HasInvitationFlag, QStringLiteral("V"));

    insertBoolProperty(IsImportantFlag);
    insertBoolProperty(IsToActFlag);
    insertBoolProperty(IsWatchedFlag);
    insertBoolProperty(IsDeletedFlag);
    insertBoolProperty(IsSpamFlag);
    insertBoolProperty(IsRepliedFlag);
    insertBoolProperty(IsIgnoredFlag);
    insertBoolProperty(IsForwardedFlag);
    insertBoolProperty(IsSentFlag);
    insertBoolProperty(IsQueuedFlag);
    insertBoolProperty(IsHamFlag);
    insertBoolProperty(IsReadFlag);
    insertBoolProperty(HasAttachmentFlag);
    insertBoolProperty(IsEncryptedFlag);
    insertBoolProperty(HasInvitationFlag);

    insertValueProperty(Akonadi::EmailSearchTerm::HeaderDate, 0);
    insertValueProperty(Akonadi::EmailSearchTerm::ByteSize, 1);
    insertValueProperty(Akonadi::EmailSearchTerm::HeaderOnlyDate, 2);
}

const EmailQueryPropertyMapper &EmailQueryPropertyMapper::instance()
{
    static QMutex lock;
    lock.lock();
    if (!sInstance) {
        sInstance = new EmailQueryPropertyMapper();
    }
    lock.unlock();
    return *sInstance;
}

