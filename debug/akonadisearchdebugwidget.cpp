/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugwidget.h"
#include "akonadisearchsyntaxhighlighter.h"
#include "job/akonadisearchdebugsearchjob.h"
#include <KLineEdit>
#include <QPushButton>

#include <QLabel>
#include <QVBoxLayout>

#include <QPlainTextEdit>

using namespace Akonadi::Search;

AkonadiSearchDebugWidget::AkonadiSearchDebugWidget(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    auto hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    auto lab = new QLabel(QStringLiteral("Item identifier:"), this);
    hbox->addWidget(lab);
    mLineEdit = new KLineEdit(this);
    mLineEdit->setTrapReturnKey(true);
    mLineEdit->setClearButtonEnabled(true);
    mLineEdit->setObjectName(QStringLiteral("lineedit"));
    connect(mLineEdit, &KLineEdit::textChanged, this, &AkonadiSearchDebugWidget::slotSearchLineTextChanged);
    hbox->addWidget(mLineEdit);

    mSearchPathComboBox = new Akonadi::Search::AkonadiSearchDebugSearchPathComboBox(this);
    hbox->addWidget(mSearchPathComboBox);
    mSearchPathComboBox->setObjectName(QStringLiteral("searchpathcombo"));

    mSearchButton = new QPushButton(QStringLiteral("Search"), this);
    mSearchButton->setObjectName(QStringLiteral("searchbutton"));
    connect(mSearchButton, &QPushButton::clicked, this, &AkonadiSearchDebugWidget::slotSearch);
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    mPlainTextEditor = new QPlainTextEdit(this);
    new AkonadiSearchSyntaxHighlighter(mPlainTextEditor->document());
    mPlainTextEditor->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEditor);
    mPlainTextEditor->setObjectName(QStringLiteral("plaintexteditor"));

    connect(mLineEdit, &KLineEdit::returnPressed, this, &AkonadiSearchDebugWidget::slotSearch);
}

AkonadiSearchDebugWidget::~AkonadiSearchDebugWidget() = default;

void AkonadiSearchDebugWidget::slotSearchLineTextChanged(const QString &text)
{
    mSearchButton->setEnabled(!text.trimmed().isEmpty());
}

void AkonadiSearchDebugWidget::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    mLineEdit->setText(QString::number(akonadiId));
}

void AkonadiSearchDebugWidget::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    mSearchPathComboBox->setSearchType(type);
}

void AkonadiSearchDebugWidget::doSearch()
{
    slotSearch();
}

QString AkonadiSearchDebugWidget::plainText() const
{
    return QStringLiteral("Item: %1\n").arg(mLineEdit->text()) + mPlainTextEditor->toPlainText();
}

void AkonadiSearchDebugWidget::slotSearch()
{
    const QString searchId = mLineEdit->text();
    if (searchId.isEmpty()) {
        return;
    }
    auto job = new Akonadi::Search::AkonadiSearchDebugSearchJob(this);
    job->setAkonadiId(searchId);
    job->setSearchPath(mSearchPathComboBox->searchPath());
    connect(job, &Akonadi::Search::AkonadiSearchDebugSearchJob::result, this, &AkonadiSearchDebugWidget::slotResult);
    connect(job, &Akonadi::Search::AkonadiSearchDebugSearchJob::error, this, &AkonadiSearchDebugWidget::slotError);
    job->start();
}

void AkonadiSearchDebugWidget::slotResult(const QString &result)
{
    mPlainTextEditor->setPlainText(result);
}

void AkonadiSearchDebugWidget::slotError(const QString &errorStr)
{
    mPlainTextEditor->setPlainText(QStringLiteral("Error found:\n") + errorStr);
}
