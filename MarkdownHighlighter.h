#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat headerFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat codeBlockFormat;
};

#endif // MARKDOWNHIGHLIGHTER_H
