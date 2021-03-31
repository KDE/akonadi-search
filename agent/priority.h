/* This file is part of the KDE Project
   SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

bool lowerIOPriority();
/// Sets the priority to batch
bool lowerSchedulingPriority();
bool setIdleSchedulingPriority();
bool lowerPriority();

