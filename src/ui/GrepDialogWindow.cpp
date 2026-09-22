#include "GrepDialogWindow.hpp"

#include "Settings.hpp"
#include "Translator.hpp"

#include <algorithm>
#include <limits>
#include <vector>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QColor>
#include <QCompleter>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QSize>
#include <QStringListModel>

namespace
{
constexpr int kSuggestionCount = 5;

int fuzzyScore(const QString& pattern, const QString& candidate)
{
    if (pattern.isEmpty())
        return 0;

    const QString needle = pattern.toCaseFolded();
    const QString haystack = candidate.toCaseFolded();

    if (haystack.startsWith(needle))
        return 100000 - (haystack.size() - needle.size());

    const qsizetype substring_index = haystack.indexOf(needle);
    if (substring_index >= 0)
        return 50000 - static_cast<int>(substring_index * 100)
               - (haystack.size() - needle.size());

    qsizetype needle_index = 0;
    qsizetype previous_match = -2;
    int score = 10000;
    for (qsizetype candidate_index = 0;
         candidate_index < haystack.size() && needle_index < needle.size();
         ++candidate_index)
    {
        if (haystack[candidate_index] != needle[needle_index])
            continue;

        if (needle_index == 0)
            score -= static_cast<int>(candidate_index * 20);
        if (candidate_index == previous_match + 1)
            score += 100;
        else if (previous_match >= 0)
            score -= static_cast<int>((candidate_index - previous_match - 1) * 10);

        previous_match = candidate_index;
        ++needle_index;
    }

    return needle_index == needle.size()
               ? score - (haystack.size() - needle.size())
               : std::numeric_limits<int>::min();
}
} // namespace

bool GrepDialogWindow::last_regex_ = false;
bool GrepDialogWindow::last_case_sensitive_ = false;
bool GrepDialogWindow::last_inverted_ = false;

GrepDialogWindow::GrepDialogWindow(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(Lang::tr("grep.title"));
    resize(400, 121);

    QGridLayout* outer = new QGridLayout(this);
    QGridLayout* inner = new QGridLayout();

    QGridLayout* patternLayout = new QGridLayout();
    patternLayout->setSpacing(1);
    QLabel* label = new QLabel(Lang::tr("grep.pattern"), this);
    label->setMinimumSize(QSize(50, 0));
    pattern_ = new QLineEdit(this);
    suggestions_model_ = new QStringListModel(this);
    completer_ = new QCompleter(suggestions_model_, this);
    completer_->setCaseSensitivity(Qt::CaseInsensitive);
    completer_->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer_->setMaxVisibleItems(kSuggestionCount);
    pattern_->setCompleter(completer_);
    patternLayout->addWidget(label, 0, 0, 1, 1);
    patternLayout->addWidget(pattern_, 0, 1, 1, 1);

    regex_check_ = new QCheckBox(Lang::tr("common.regex"), this);
    case_sensitive_check_ = new QCheckBox(Lang::tr("common.case_sensitive"), this);
    inverted_check_ = new QCheckBox(Lang::tr("grep.inverted"), this);
    inverted_check_->setFocusPolicy(Qt::StrongFocus);
    button_ = new QPushButton(Lang::tr("grep.title"), this);

    inner->addLayout(patternLayout, 0, 0, 1, 1);
    inner->addWidget(regex_check_, 1, 0, 1, 1);
    inner->addWidget(case_sensitive_check_, 2, 0, 1, 1);
    inner->addWidget(inverted_check_, 3, 0, 1, 1);
    inner->addWidget(button_, 5, 0, 1, 1);

    outer->addLayout(inner, 1, 0, 1, 1);

    setTabOrder(pattern_, regex_check_);
    setTabOrder(regex_check_, case_sensitive_check_);
    setTabOrder(case_sensitive_check_, button_);

    regex_check_->setChecked(last_regex_);
    case_sensitive_check_->setChecked(last_case_sensitive_);
    inverted_check_->setChecked(last_inverted_);

    connect(button_, &QPushButton::clicked, this, &GrepDialogWindow::on_button_clicked);
    connect(regex_check_, &QCheckBox::clicked, this, &GrepDialogWindow::on_regex_check_clicked);
    connect(pattern_, &QLineEdit::textEdited, this, &GrepDialogWindow::on_pattern_textEdited);

    on_regex_check_clicked();
}

GrepDialogWindow::~GrepDialogWindow() = default;

GrepDialogWindow::Result GrepDialogWindow::getResult()
{
    Result result;
    result.pattern = pattern_->text();
    result.is_regex = regex_check_->isChecked();
    result.is_case_sensitive = case_sensitive_check_->isChecked();
    result.is_inverted = inverted_check_->isChecked();
    return result;
}

void GrepDialogWindow::on_button_clicked()
{
    last_regex_ = regex_check_->isChecked();
    last_case_sensitive_ = case_sensitive_check_->isChecked();
    last_inverted_ = inverted_check_->isChecked();
    Settings::instance().addRecentGrep(pattern_->text());
    accept();
}

void GrepDialogWindow::on_regex_check_clicked()
{
    if (regex_check_->isChecked())
    {
        case_sensitive_check_->setEnabled(false);
        on_pattern_textEdited(pattern_->text());
    }
    if (!regex_check_->isChecked())
    {
        case_sensitive_check_->setEnabled(true);
        pattern_->setPalette(QPalette());
    }
}

void GrepDialogWindow::on_pattern_textEdited(const QString &arg1)
{
    updateSuggestions(arg1);

    if (regex_check_->isChecked())
    {
        QPalette pallete = pattern_->palette();
        QRegularExpression expression(arg1);
        const QColor background = expression.isValid()
                                      ? QColor(Qt::green).lighter()
                                      : QColor(Qt::red).lighter();
        pallete.setColor(QPalette::Base, background);
        pallete.setColor(QPalette::Text, Qt::black);
        pattern_->setPalette(pallete);
    }
}

void GrepDialogWindow::updateSuggestions(const QString& pattern)
{
    struct Match
    {
        QString pattern;
        int score;
        int recency;
    };

    std::vector<Match> matches;
    const QStringList history = Settings::instance().recentGreps();
    matches.reserve(static_cast<std::size_t>(history.size()));
    for (int i = 0; i < history.size(); ++i)
    {
        const int score = fuzzyScore(pattern, history[i]);
        if (score != std::numeric_limits<int>::min())
            matches.push_back({history[i], score, i});
    }

    std::stable_sort(matches.begin(), matches.end(),
                     [](const Match& left, const Match& right)
                     {
                         if (left.score != right.score)
                             return left.score > right.score;
                         return left.recency < right.recency;
                     });

    QStringList suggestions;
    const int count = std::min(kSuggestionCount,
                               static_cast<int>(matches.size()));
    for (int i = 0; i < count; ++i)
        suggestions.append(matches[static_cast<std::size_t>(i)].pattern);

    suggestions_model_->setStringList(suggestions);
    completer_->setCompletionPrefix(QString());
    if (suggestions.isEmpty())
        completer_->popup()->hide();
    else
        completer_->complete();
}
