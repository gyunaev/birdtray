#include <QtCore/QLibraryInfo>
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
        MorkParser::dumpMorkFile(morkPath);
        exit(1); // TODO: Why 1? Replace with return code of dumpMorkFile
        return;
    }
    QString imapString = parser.value("decode");
    if (!imapString.isEmpty()) {
        printf("Decoded: %s\n", qPrintable(Utils::decodeIMAPutf7(imapString)));
        exit(1); // TODO: Why 1? Replace with EXIT_SUCCESS
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
    return QApplication::event(event);
}

bool BirdtrayApp::loadTranslations() {
    QString translationDir = QCoreApplication::applicationDirPath() + "/translations";
    bool success = qtTranslator.load(
            QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    success &= !dynamicTranslator.load(QLocale::system(), "dynamic", "_", translationDir);
    success |= !mainTranslator.load(QLocale::system(), "main", "_", translationDir);
    return success;
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
    // If system tray is not yet available, wait up to 60 seconds
    int passed = 0;
    while ( !QSystemTrayIcon::isSystemTrayAvailable() ) {
        if ( passed == 0 ) {
            qDebug("Waiting for system tray to become available");
        }
        passed++;
        if ( passed > 120 ) {
            Utils::fatal(QApplication::tr("Sorry, system tray cannot be controlled "
                                          "through this add-on on your operating system."));
        }
        QThread::msleep( 500 );
    }
}
