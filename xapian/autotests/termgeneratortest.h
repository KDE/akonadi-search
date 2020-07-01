/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef TERMGENERATORTEST_H
#define TERMGENERATORTEST_H

#include <QObject>

class TermGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWordBoundaries();
    void testUnderscore_splitting();
    void testAccetCharacters();
    void testUnicodeCompatibleComposition();
    void testEmails();
    void testWordPositions();
};

#endif // TERMGENERATORTEST_H
