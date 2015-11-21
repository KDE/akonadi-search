/*
 * Copyright (C) 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef PIMCONTACTSRUNNERCONFIG_H
#define PIMCONTACTSRUNNERCONFIG_H

#include <KCModule>

class QCheckBox;

class PIMContactsRunnerConfig : public KCModule
{
    Q_OBJECT
public:
    explicit PIMContactsRunnerConfig(QWidget *parent = Q_NULLPTR,
                                     const QVariantList &args = QVariantList());

public Q_SLOTS:
    void save();
    void load();
    void defaults();

private:
    QCheckBox *mQueryCompletionCheckBox;
};

#endif