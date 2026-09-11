/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>
namespace Akonadi
{
namespace Search
{
class AkonadiSearchDebugSearchJob : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiSearchDebugSearchJob(QObject *parent = nullptr);
    ~AkonadiSearchDebugSearchJob() override;

    void start();

    void setAkonadiId(const QString &id);
    void setArguments(const QStringList &args);
    void setSearchPath(const QString &path);

Q_SIGNALS:
    void error(const QString &errorString);
    void result(const QString &text);

private:
    void slotReadStandard();
    void slotReadError();
    void slotFinished();
    void slotErrorOccurred(QProcess::ProcessError error);
    void deleteProcessAndJob();
    QByteArray mStandardOutput;
    QByteArray mErrorOutput;
    QStringList mArguments;
    QString mAkonadiId;
    QString mPath;
    QProcess *mProcess = nullptr;
};
}
}
