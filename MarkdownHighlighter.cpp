#include "MarkdownHighlighter.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Bold format
    boldFormat.setFontWeight(QFont::Bold);
    HighlightingRule boldRule;
    boldRule.pattern = QRegularExpression("\\*\\*(.*?)\\*\\*");
    boldRule.format = boldFormat;
    highlightingRules.append(boldRule);

    // Italic format
    italicFormat.setFontItalic(true);
    HighlightingRule italicRule;
    italicRule.pattern = QRegularExpression("\\*(.*?)\\*");
    italicRule.format = italicFormat;
    highlightingRules.append(italicRule);

    // Header format
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setForeground(Qt::blue);
    for (int i = 1; i <= 6; ++i) {
        HighlightingRule headerRule;
        headerRule.pattern = QRegularExpression(QString("#{%1}\\s+(.*)").arg(i));
        headerRule.format = headerFormat;
        highlightingRules.append(headerRule);
    }

    // Link format
    linkFormat.setForeground(Qt::blue);
    linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    HighlightingRule linkRule;
    linkRule.pattern = QRegularExpression("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    linkRule.format = linkFormat;
    highlightingRules.append(linkRule);

    // Code format
    codeFormat.setForeground(Qt::darkGreen);
    HighlightingRule codeRule;
    codeRule.pattern = QRegularExpression("`([^`]+)`");
    codeRule.format = codeFormat;
    highlightingRules.append(codeRule);

    // Code block format
    codeBlockFormat.setForeground(Qt::darkGreen);
    codeBlockFormat.setBackground(QColor(230, 230, 230));
    HighlightingRule codeBlockRule;
    codeBlockRule.pattern = QRegularExpression("```([^`]+)```");
    codeBlockRule.format = codeBlockFormat;
    highlightingRules.append(codeBlockRule);
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
