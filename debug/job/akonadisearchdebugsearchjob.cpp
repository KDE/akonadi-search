/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

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
        connect(mProcess, &QProcess::errorOccurred, this, &AkonadiSearchDebugSearchJob::slotErrorOccurred);
        connect(mProcess, &QProcess::finished, this, &AkonadiSearchDebugSearchJob::slotFinished);
        mProcess->setWorkingDirectory(mPath);
        QStringList arguments;
        arguments << u"-r"_s << mAkonadiId;
        arguments << mPath;
        mProcess->start(delvePath, arguments);
    }
}

void AkonadiSearchDebugSearchJob::deleteProcessAndJob()
{
    mProcess->close();
    mProcess->deleteLater();
    mProcess = nullptr;
    deleteLater();
}

void AkonadiSearchDebugSearchJob::slotErrorOccurred(QProcess::ProcessError processError)
{
    if (processError == QProcess::FailedToStart) {
        // Don't translate it. Just debug
        Q_EMIT error(u"Unable to start \"delve\": "_s + mProcess->errorString());
        deleteProcessAndJob();
    }
}

void AkonadiSearchDebugSearchJob::slotFinished()
{
    // Read what is still buffered before closing the process.
    mStandardOutput += mProcess->readAllStandardOutput();
    mErrorOutput += mProcess->readAllStandardError();
    if (!mStandardOutput.isEmpty()) {
        Q_EMIT result(QString::fromUtf8(mStandardOutput));
    } else if (!mErrorOutput.isEmpty()) {
        Q_EMIT error(QString::fromUtf8(mErrorOutput));
    } else {
        // Don't translate it. Just debug
        Q_EMIT error(u"\"delve\" didn't return any output."_s);
    }
    deleteProcessAndJob();
}

void AkonadiSearchDebugSearchJob::slotReadStandard()
{
    mStandardOutput += mProcess->readAllStandardOutput();
}

void AkonadiSearchDebugSearchJob::slotReadError()
{
    mErrorOutput += mProcess->readAllStandardError();
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
