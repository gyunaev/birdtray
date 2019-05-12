#include <QApplication>
#include <QtCore/QCommandLineParser>

#ifdef Q_OS_WIN
#include "birdtrayeventfilter.h"
#endif /* Q_OS_WIN */
#include "dialogsettings.h"
#include "trayicon.h"
#include "settings.h"
#include "morkparser.h"
#include "utils.h"
#include "version.h"


void ensureSystemTrayAvailable() {
    // If system tray is not yet available, wait up to 60 seconds
    int passed = 0;
    while ( !QSystemTrayIcon::isSystemTrayAvailable() ) {
        if ( passed == 0 ) {
            qDebug("Waiting for system tray to become available");
        }
        passed++;
        if ( passed > 120 ) {
            Utils::fatal("Sorry, system tray cannot be controlled "
                         "through this addon on your operating system");
        }
        QThread::usleep( 500 );
    }
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(QString::fromUtf8(":/res/birdtray.ico")));
    QCoreApplication::setOrganizationName("ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("birdtray");
    QCoreApplication::setApplicationVersion(QString("%1.%2").arg(VERSION_MAJOR).arg(VERSION_MINOR));
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
    pSettings->load();

    TrayIcon trayIcon;
    return QApplication::exec();
}
