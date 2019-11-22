#include <QtCore/QLibraryInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QThread>
#include "birdtrayapp.h"
#ifdef Q_OS_WIN
#  include "birdtrayeventfilter.h"
#endif /* Q_OS_WIN */
#include "utils.h"
#include "morkparser.h"


BirdtrayApp::BirdtrayApp(int &argc, char** argv) : QApplication(argc, argv) {
    QApplication::setWindowIcon(QIcon(QString::fromUtf8(":/res/birdtray.ico")));
    QCoreApplication::setOrganizationName("ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("birdtray");
    QCoreApplication::setApplicationVersion(Utils::getBirdtrayVersion());
    // This prevents exiting the application when the dialogs are closed on Gnome/XFCE
    QApplication::setQuitOnLastWindowClosed(false);
    
    bool translationLoadedSuccessfully = loadTranslations();
    QCoreApplication::installTranslator(&qtTranslator);
    QCoreApplication::installTranslator(&dynamicTranslator);
    QCoreApplication::installTranslator(&mainTranslator);

#ifdef Q_OS_WIN
    BirdtrayEventFilter filter;
    installNativeEventFilter(&filter);
#endif /* Q_OS_WIN */
    
    QCommandLineParser parser;
    parseCmdArguments(parser);
    
    QString morkPath = parser.value("dump-mork");
    if (!morkPath.isEmpty()) {
        exit(MorkParser::dumpMorkFile(morkPath));
        return;
    }
    QString imapString = parser.value("decode");
    if (!imapString.isEmpty()) {
        printf("Decoded: %s\n", qPrintable(Utils::decodeIMAPutf7(imapString)));
        exit(EXIT_SUCCESS);
        return;
    }
    
    ensureSystemTrayAvailable();
    
    // Load settings
    settings = new Settings(parser.isSet("debug"));
    if (parser.isSet("reset-settings")) {
        settings->save(); // Saving without loading will reset the values
    } else {
        settings->load();
    }
    if (!translationLoadedSuccessfully) {
        Utils::debug("Failed to load translation for %s", qPrintable(QLocale::system().name()));
    }
    autoUpdater = new AutoUpdater();
    trayIcon = new TrayIcon(parser.isSet("settings"));
}

BirdtrayApp::~BirdtrayApp() {
    delete trayIcon;
    delete autoUpdater;
    delete settings;
}

BirdtrayApp* BirdtrayApp::get() {
    return dynamic_cast<BirdtrayApp*>(QCoreApplication::instance());
}

Settings* BirdtrayApp::getSettings() const {
    return settings;
}

AutoUpdater* BirdtrayApp::getAutoUpdater() const {
    return autoUpdater;
}

TrayIcon* BirdtrayApp::getTrayIcon() const {
    return trayIcon;
}

bool BirdtrayApp::event(QEvent* event) {
    if (event->type() == QEvent::LocaleChange) {
        if (!loadTranslations()) {
            Utils::debug("Failed to load translation for %s", qPrintable(QLocale::system().name()));
        }
        return true;
    }
    return QApplication::event(event);
}

bool BirdtrayApp::loadTranslations() {
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    locations.prepend(QCoreApplication::applicationDirPath());
    std::transform(locations.begin(), locations.end(), locations.begin(),
            [](QString path) { return path.append("/translations"); });
    QLocale locale = QLocale::system();
    bool success = loadTranslation(
            qtTranslator, locale, "qt", {QLibraryInfo::location(QLibraryInfo::TranslationsPath)});
    success &= loadTranslation(dynamicTranslator, locale, "dynamic", locations);
    success &= loadTranslation(mainTranslator, locale, "main", locations);
    return success;
}

bool BirdtrayApp::loadTranslation(QTranslator &translator, QLocale &locale,
                                  const QString &translationName, const QStringList &paths) {
    QLocale languageWithoutCountry(locale.language());
    for (const QString &path : paths) {
        if (translator.load(locale, translationName, "_", path)) {
            return true;
        }
        // On Ubuntu, when switching to another language, the LANGUAGE environment variable
        // does not include the base language, only the language with country,
        // e.g. LANGUAGE=de_DE:en_US:en
        // instead of LANGUAGE=de_DE:de:en_US:en
        // As a result, the translator does not find the <translationName>_de.qm files.
        // That's why we try to load the translation without the country appendix.
        if (translator.load(languageWithoutCountry, translationName, "_", path)) {
            return true;
        }
    }
    return false;
}

void BirdtrayApp::parseCmdArguments(QCommandLineParser &parser) {
    parser.setApplicationDescription(
            tr("A free system tray notification for new mail for Thunderbird"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
            {"dump-mork", tr("Display the contents of the given mork database."),
             tr("databaseFile")},
            {"decode", tr("Decode an IMAP Utf7 string."), tr("string")},
            {"settings", tr("Show the settings.")},
            {{"r", "reset-settings"}, tr("Reset the settings to the defaults.")},
            {{"d", "debug"}, tr("Enable debugging output.")},
    });
    parser.process(*this);
}

void BirdtrayApp::ensureSystemTrayAvailable() {
    int passed = 0;
    while (!QSystemTrayIcon::isSystemTrayAvailable()) {
        if (passed == 0) {
            qDebug("Waiting for system tray to become available");
        }
        passed++;
        if (passed > 120) {
            Utils::fatal(QApplication::tr("Sorry, system tray cannot be controlled "
                                          "through this add-on on your operating system."));
        }
        QThread::msleep(500);
    }
}
