#include <QApplication>
#include <QTranslator>
#include <QtCore/QCommandLineParser>

#ifdef Q_OS_WIN
#include "birdtrayeventfilter.h"
#endif /* Q_OS_WIN */
#include "dialogsettings.h"
#include "trayicon.h"
#include "settings.h"
#include "morkparser.h"
#include "utils.h"
#include "autoupdater.h"


void ensureSystemTrayAvailable() {
    // If system tray is not yet available, wait up to 60 seconds
    int passed = 0;
    while ( !QSystemTrayIcon::isSystemTrayAvailable() ) {
        if ( passed == 0 ) {
            qDebug("Waiting for system tray to become available");
        }
        passed++;
        if ( passed > 120 ) {
            Utils::fatal(QApplication::tr("Sorry, system tray cannot be controlled "
                                          "through this addon on your operating system."));
        }
        QThread::msleep( 500 );
    }
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(QString::fromUtf8(":/res/birdtray.ico")));
    QCoreApplication::setOrganizationName("ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("birdtray");
    QCoreApplication::setApplicationVersion(Utils::getBirdtrayVersion());
    QTranslator translator;
    translator.load(QCoreApplication::applicationDirPath()
                    + "/translations/qt_" + QLocale::system().name());
    bool translationLoadFailed = !translator.load(QCoreApplication::applicationDirPath() +
                                                  "/translations/main_" + QLocale::system().name());
    QCoreApplication::installTranslator(&translator);
#ifdef Q_OS_WIN
    BirdtrayEventFilter filter;
    app.installNativeEventFilter(&filter);
#endif /* Q_OS_WIN */
    
    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr(
            "A free system tray notification for new mail for Thunderbird"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {"dump-mork", QApplication::tr("Display the contents of the given mork database."),
         QApplication::tr("databaseFile")},
        {"decode", QApplication::tr("Decode an IMAP Utf7 string."), QApplication::tr("string")},
        {"settings", QApplication::tr("Show the settings.")},
        {{"r", "reset-settings"}, QApplication::tr("Reset the settings to the defaults.")},
        {{"d", "debug"}, QApplication::tr("Enable debugging output.")},
    });
    parser.process(app);
    
    QString morkPath = parser.value("dump-mork");
    if ( !morkPath.isEmpty() )
    {
        MorkParser::dumpMorkFile( morkPath );
        return 1;
    }
    
    QString imapString = parser.value("decode");
    if ( !imapString.isEmpty() )
    {
        printf( "Decoded: %s\n", qPrintable( Utils::decodeIMAPutf7( imapString )));
        return 1;
    }
    
    ensureSystemTrayAvailable();
    
    // This prevents exiting the application when the dialogs are closed on Gnome/XFCE
    QApplication::setQuitOnLastWindowClosed( false );

    // Load settings
    pSettings = new Settings(parser.isSet("debug"));
    if (parser.isSet("reset-settings")) {
        pSettings->save(); // Saving without loading will reset the values
    } else {
        pSettings->load();
    }
    if (translationLoadFailed) {
        Utils::debug("Failed to load translation for %s", qPrintable(QLocale::system().name()));
    }
    autoUpdaterSingleton = new AutoUpdater();

    TrayIcon trayIcon(parser.isSet("settings"));
    int result = QApplication::exec();

    delete pSettings;
    delete autoUpdaterSingleton;

    return result;
}
