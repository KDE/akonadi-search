/*
 * SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QObject>
using namespace Qt::Literals::StringLiterals;

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
