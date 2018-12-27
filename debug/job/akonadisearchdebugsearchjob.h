/*
  Copyright (c) 2014-2019 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef AKONADISEARCHDEBUGSEARCHJOB_H
#define AKONADISEARCHDEBUGSEARCHJOB_H

#include <QObject>
#include <QStringList>
class QProcess;
namespace Akonadi
{
namespace Search
{
class AkonadiSearchDebugSearchJob : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugSearchJob(QObject *parent = nullptr);
    ~AkonadiSearchDebugSearchJob();

    void start();

    void setAkonadiId(const QString &id);
    void setArguments(const QStringList &args);
    void setSearchPath(const QString &path);

Q_SIGNALS:
    void error(const QString &errorString);
    void result(const QString &text);

private Q_SLOTS:
    void slotReadStandard();
    void slotReadError();

private:
    QStringList mArguments;
    QString mAkonadiId;
    QString mPath;
    QProcess *mProcess = nullptr;
};
}
}
#endif // AKONADISEARCHDEBUGSEARCHJOB_H

