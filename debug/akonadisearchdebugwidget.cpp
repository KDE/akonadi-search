/*
  SPDX-FileCopyrightText: 2014-2024 Laurent Montel <montel@kde.org>

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
    , mPlainTextEditor(new QPlainTextEdit(this))
    , mSearchPathComboBox(new Akonadi::Search::AkonadiSearchDebugSearchPathComboBox(this))
    , mLineEdit(new KLineEdit(this))
    , mSearchButton(new QPushButton(QStringLiteral("Search"), this))
{
    auto mainLayout = new QVBoxLayout(this);

    auto hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    auto lab = new QLabel(QStringLiteral("Item identifier:"), this);
    hbox->addWidget(lab);
    mLineEdit->setTrapReturnKey(true);
    mLineEdit->setClearButtonEnabled(true);
    mLineEdit->setObjectName(QLatin1StringView("lineedit"));
    connect(mLineEdit, &KLineEdit::textChanged, this, &AkonadiSearchDebugWidget::slotSearchLineTextChanged);
    hbox->addWidget(mLineEdit);

    hbox->addWidget(mSearchPathComboBox);
    mSearchPathComboBox->setObjectName(QLatin1StringView("searchpathcombo"));

    mSearchButton->setObjectName(QLatin1StringView("searchbutton"));
    connect(mSearchButton, &QPushButton::clicked, this, &AkonadiSearchDebugWidget::slotSearch);
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    new AkonadiSearchSyntaxHighlighter(mPlainTextEditor->document());
    mPlainTextEditor->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEditor);
    mPlainTextEditor->setObjectName(QLatin1StringView("plaintexteditor"));

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

#include "moc_akonadisearchdebugwidget.cpp"
