/*
  Copyright (c) 2014-2020 Laurent Montel <montel@kde.org>

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

#include "akonadisearchdebugsearchjob.h"

#include <QProcess>
#include <QStandardPaths>

using namespace Akonadi::Search;
AkonadiSearchDebugSearchJob::AkonadiSearchDebugSearchJob(QObject *parent)
    : QObject(parent)
    , mProcess(nullptr)
{
}

AkonadiSearchDebugSearchJob::~AkonadiSearchDebugSearchJob()
{
}

void AkonadiSearchDebugSearchJob::start()
{
    // "delve" is also a name of Go debugger, some distros prefer xapian-delve
    // for that reason, so try that first and fallback to "delve"
    QString delvePath = QStandardPaths::findExecutable(QStringLiteral("xapian-delve"));
    if (delvePath.isEmpty()) {
        delvePath = QStandardPaths::findExecutable(QStringLiteral("delve"));
    }
    if (delvePath.isEmpty()) {
        //Don't translate it. Just debug
        Q_EMIT error(QStringLiteral("\"delve\" not installed on computer."));
        deleteLater();
        return;
    } else {
        mProcess = new QProcess(this);
        connect(mProcess, &QProcess::readyReadStandardOutput, this, &AkonadiSearchDebugSearchJob::slotReadStandard);
        connect(mProcess, &QProcess::readyReadStandardError, this, &AkonadiSearchDebugSearchJob::slotReadError);
        mProcess->setWorkingDirectory(mPath);
        QStringList arguments;
        arguments << QStringLiteral("-r") << mAkonadiId;
        arguments << mPath;
        mProcess->start(delvePath, QStringList() << arguments);
    }
}

void AkonadiSearchDebugSearchJob::slotReadStandard()
{
    const QByteArray stdStrg = mProcess->readAllStandardOutput();
    Q_EMIT result(QString::fromUtf8(stdStrg));
    mProcess->close();
    mProcess->deleteLater();
    mProcess = nullptr;
    deleteLater();
}

void AkonadiSearchDebugSearchJob::slotReadError()
{
    const QByteArray errorStrg = mProcess->readAllStandardOutput();
    Q_EMIT error(QString::fromUtf8(errorStrg));
    mProcess->close();
    mProcess->deleteLater();
    mProcess = nullptr;
    deleteLater();
}

void AkonadiSearchDebugSearchJob::setAkonadiId(const QString &id)
{
    mAkonadiId = id;
}

void AkonadiSearchDebugSearchJob::setArguments(const QStringList &args)
{
    mArguments = args;
}

void AkonadiSearchDebugSearchJob::setSearchPath(const QString &path)
{
    mPath = path;
}
