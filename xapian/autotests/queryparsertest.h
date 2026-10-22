/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QObject>

class QueryParserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSinglePrefixWord();
    void testSimpleQuery();
    void testPhraseSearch();
    void testPhraseSearchOnly();
    void testPhraseSearch_sameLimiter();
    void testPhraseSearchEmail();
    void testAccentSearch();
    void testUnderscoreSplitting();

    void testWordExpansion();
};
