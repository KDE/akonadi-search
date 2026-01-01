/*
  SPDX-FileCopyrightText: 2014-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akonadisearchdebugwidget.h"

#include "akonadisearchsyntaxhighlighter.h"
#include "job/akonadisearchdebugsearchjob.h"
#include <KLineEditEventHandler>
#include <QLineEdit>
#include <QPushButton>

#include <QLabel>
#include <QVBoxLayout>

#include <QPlainTextEdit>

using namespace Qt::Literals::StringLiterals;
using namespace Akonadi::Search;

AkonadiSearchDebugWidget::AkonadiSearchDebugWidget(QWidget *parent)
    : QWidget(parent)
    , mPlainTextEditor(new QPlainTextEdit(this))
    , mSearchPathComboBox(new Akonadi::Search::AkonadiSearchDebugSearchPathComboBox(this))
    , mLineEdit(new QLineEdit(this))
    , mSearchButton(new QPushButton(u"Search"_s, this))
{
    auto mainLayout = new QVBoxLayout(this);

    auto hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    auto lab = new QLabel(u"Item identifier:"_s, this);
    hbox->addWidget(lab);
    KLineEditEventHandler::catchReturnKey(mLineEdit);
    mLineEdit->setClearButtonEnabled(true);
    mLineEdit->setObjectName("lineedit"_L1);
    connect(mLineEdit, &QLineEdit::textChanged, this, &AkonadiSearchDebugWidget::slotSearchLineTextChanged);
    hbox->addWidget(mLineEdit);

    hbox->addWidget(mSearchPathComboBox);
    mSearchPathComboBox->setObjectName("searchpathcombo"_L1);

    mSearchButton->setObjectName("searchbutton"_L1);
    connect(mSearchButton, &QPushButton::clicked, this, &AkonadiSearchDebugWidget::slotSearch);
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    new AkonadiSearchSyntaxHighlighter(mPlainTextEditor->document());
    mPlainTextEditor->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEditor);
    mPlainTextEditor->setObjectName("plaintexteditor"_L1);

    connect(mLineEdit, &QLineEdit::returnPressed, this, &AkonadiSearchDebugWidget::slotSearch);
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
    return u"Item: %1\n"_s.arg(mLineEdit->text()) + mPlainTextEditor->toPlainText();
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
    mPlainTextEditor->setPlainText(u"Error found:\n"_s + errorStr);
}

#include "moc_akonadisearchdebugwidget.cpp"
