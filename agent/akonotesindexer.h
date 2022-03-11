/*
 * This file is part of the KDE Akonadi Search Project
 * SPDX-FileCopyrightText: 2014-2022 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include <xapian.h>

#include "abstractindexer.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KMime/Message>

class AkonotesIndexer : public AbstractIndexer
{
public:
    /**
     * You must provide the path where the indexed information
     * should be stored
     */
    explicit AkonotesIndexer(const QString &path);
    ~AkonotesIndexer() override;

    Q_REQUIRED_RESULT QStringList mimeTypes() const override;

    void index(const Akonadi::Item &item) override;
    void commit() override;

    void remove(const Akonadi::Item &item) override;
    void remove(const Akonadi::Collection &collection) override;
    void move(Akonadi::Item::Id itemId, Akonadi::Collection::Id from, Akonadi::Collection::Id to) override;

private:
    void processPart(KMime::Content *content, KMime::Content *mainContent);
    void process(const KMime::Message::Ptr &msg);
    Xapian::WritableDatabase *m_db = nullptr;
    Xapian::Document *m_doc = nullptr;
    Xapian::TermGenerator *m_termGen = nullptr;
};

