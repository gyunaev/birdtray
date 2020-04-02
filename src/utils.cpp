#include "utils.h"
#include "version.h"
#include "birdtrayapp.h"

#include <QApplication>
#include <QTextCodec>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>

#if defined (Q_OS_WIN)
#  include <windows.h>
#elif defined (Q_OS_MAC)
#else
#  include <wordexp.h>
#endif


QString Utils::expandPath(const QString &path) {
    QString rawPath;
    if (path.startsWith('"') && path.endsWith('"')) {
        rawPath = path.section('"', 1, 1);
    } else {
        rawPath = path;
    }
#if defined (Q_OS_WIN)
    TCHAR buffer[MAX_PATH];
#if defined(UNICODE)
    std::wstring originalPath = qToStdWString(rawPath);
#else
    std::string originalPath = rawPath.toStdString();
#endif /* defined(UNICODE) */
    DWORD resultSize = ExpandEnvironmentStrings(
            originalPath.data(), buffer, sizeof(buffer) / sizeof(buffer[0]));
    if (resultSize == 0 || resultSize > sizeof(buffer) / sizeof(buffer[0])) {
        return rawPath;
    }
#if defined(UNICODE)
    return QString::fromWCharArray(buffer, static_cast<int>(resultSize - 1));
#else
    return QString(buffer);
#endif /* defined(UNICODE) */

#elif defined (Q_OS_MAC)
    return rawPath;
#else
    wordexp_t result;
    if (wordexp(rawPath.toStdString().data(), &result, WRDE_NOCMD) != 0) {
        return rawPath;
    }
    QString expandedPath(result.we_wordv[0]);
    wordfree(&result);
    return expandedPath;
#endif
}

QString Utils::getBirdtrayVersion() {
    return QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH);
}

QString Utils::stdWToQString(const std::wstring &str) {
#ifdef _MSC_VER
    return QString::fromUtf16((const ushort*) str.c_str());
#else
    return QString::fromStdWString(str);
#endif
}

std::wstring Utils::qToStdWString(const QString &str) {
#ifdef _MSC_VER
    return std::wstring((const wchar_t*) str.utf16());
#else
    return str.toStdWString();
#endif
}

QStringList Utils::getThunderbirdProfilesPaths() {
#if defined (OPT_THUNDERBIRD_PROFILE)
    return { OPT_THUNDERBIRD_PROFILE };
#elif defined (Q_OS_WIN)
    return {"%APPDATA%\\Thunderbird\\Profiles"};
#elif defined (Q_OS_MAC)
    return {"~/Library/Thunderbird/Profiles"};
#else // Linux
    return {"~/.thunderbird", "~/snap/thunderbird/common/.thunderbird", "~/.var/app/org.mozilla.Thunderbird/.thunderbird"};
#endif /* Platform */
}

QStringList Utils::getDefaultThunderbirdCommand() {
#if defined (OPT_THUNDERBIRD_CMDLINE)
    return Utils::splitCommandLine( OPT_THUNDERBIRD_CMDLINE );
#elif defined (Q_OS_WIN)
    return {R"("%ProgramFiles(x86)%\Mozilla Thunderbird\thunderbird.exe")"};
#else
    return { "/usr/bin/thunderbird" };
#endif
}

QStringList Utils::splitCommandLine(const QString &src)
{
    // This must handle strings like "C:\Program Files\exe\thunderbird.exe" --profile test --title "My test profile" test "a 'bb' \"V\" c"
    // must return an array with 7 elements
    QChar sep = QChar::Null;
    QStringList out;
    QString current;

    for ( int i = 0; i < src.length(); i++ )
    {
        QChar ch = src[i];

        // If we're inside a separator, we reset sep if we encounter a separator.
        // This ensures things like "aa 'bb' cc" are handled properly
        if ( sep != QChar::Null && ch == sep )
        {
            sep = QChar::Null;
            continue;
        }
        else if ( sep == QChar::Null && (ch == '\'' || ch == '"') )
        {
            sep = ch;
            continue;
        }

        // Space separates strings, but only if we're not inside a separator
        if ( sep == QChar::Null && ch.isSpace() )
        {
            out.push_back( current );
            current.clear();
        }
        else
            current.append( ch );
    }

    if ( !current.isEmpty() )
        out.push_back( current );

    return out;
}

QString Utils::getMailFolderName(const QFileInfo &morkFile) {
    QString dirName;
    QDir parentDir = morkFile.dir();
    QString name = QCoreApplication::translate(
            "EmailFolders", morkFile.completeBaseName().toUtf8().constData());
    while ((dirName = parentDir.dirName()).endsWith(".sbd")) {
        dirName.chop(4);
        name = QCoreApplication::translate(
                "EmailFolders", dirName.toUtf8().constData()) + '/' + name;
        parentDir.cdUp();
    }
    return name;
}

QString Utils::getMailAccountName(const QFileInfo &morkFile) {
    QDir parentDir = morkFile.dir();
    QString name;
    while ((name = parentDir.dirName()).endsWith(".sbd")) {
        parentDir.cdUp();
    }
    return name;
}

QString Utils::formatGithubMarkdown(const QString& markdown) {
    QString input = markdown;
    static QRegularExpression githubMentionRegex(R"(((?<![^\s\n]))@(\S+))");
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return input.replace(githubMentionRegex, R"(\1[@\2](https://github.com/\2))");
#else
    static QRegularExpression markdownLinksRegex(R"(\[(.+?)\]\((\S+?)\))");
    return input.replace(githubMentionRegex, R"(@\1\2 (https://github.com/\2))")
                .replace(markdownLinksRegex, R"(\1 (\2))");
#endif
}

