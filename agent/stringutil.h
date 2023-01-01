/*
 * SPDX-FileCopyrightText: 2022-2023 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once
#include <QString>

namespace StringUtil
{
Q_REQUIRED_RESULT QString normalize(QStringView str);
};
