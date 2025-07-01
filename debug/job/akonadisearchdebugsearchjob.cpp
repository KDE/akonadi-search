/*
  SPDX-FileCopyrightText: 2014-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugsearchjob.h"
using namespace Qt::Literals::StringLiterals;

#include <QProcess>
#include <QStandardPaths>

using namespace Akonadi::Search;
AkonadiSearchDebugSearchJob::AkonadiSearchDebugSearchJob(QObject *parent)
    : QObject(parent)
{
}

AkonadiSearchDebugSearchJob::~AkonadiSearchDebugSearchJob() = default;

void AkonadiSearchDebugSearchJob::start()
{
    // "delve" is also a name of Go debugger, some distros prefer xapian-delve
    // for that reason, so try that first and fallback to "delve"
    QString delvePath = QStandardPaths::findExecutable(u"xapian-delve"_s);
    if (delvePath.isEmpty()) {
        delvePath = QStandardPaths::findExecutable(u"delve"_s);
    }
    if (delvePath.isEmpty()) {
        // Don't translate it. Just debug
        Q_EMIT error(u"\"delve\" not installed on computer."_s);
        deleteLater();
        return;
    } else {
        mProcess = new QProcess(this);
        connect(mProcess, &QProcess::readyReadStandardOutput, this, &AkonadiSearchDebugSearchJob::slotReadStandard);
        connect(mProcess, &QProcess::readyReadStandardError, this, &AkonadiSearchDebugSearchJob::slotReadError);
        mProcess->setWorkingDirectory(mPath);
        QStringList arguments;
        arguments << u"-r"_s << mAkonadiId;
        arguments << mPath;
        mProcess->start(delvePath, arguments);
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

#include "moc_akonadisearchdebugsearchjob.cpp"
