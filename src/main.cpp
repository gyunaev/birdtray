#include <QApplication>
#include <QAbstractNativeEventFilter>
#ifdef Q_OS_WIN
#include <windows.h>
#endif /* Q_OS_WIN */

#include "dialogsettings.h"
#include "trayicon.h"
#include "settings.h"
#include "morkparser.h"
#include "utils.h"


#ifdef Q_OS_WIN
/**
 * Filter to handle WM_CLOSE events on Windows.
 */
class BirdtrayEventFilter: public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override {
        if (eventType == "windows_generic_MSG") {
            MSG* messageEvent = static_cast<MSG*>(message);
            if (messageEvent->message == WM_CLOSE) {
                QApplication::quit();
                if (result != nullptr) {
                    *result = 0;
                }
                return true;
            }
        }
        return false;
    }
};
#endif /* Q_OS_WIN */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(QString::fromUtf8(":/res/birdtray.ico")));
#ifdef Q_OS_WIN
    BirdtrayEventFilter filter;
    a.installNativeEventFilter(&filter);
#endif /* Q_OS_WIN */

    if ( argc == 3 && !strcmp( argv[1], "--dumpmork" ) )
    {
        MorkParser::dumpMorkFile( argv[2] );
        return 1;
    }

    if ( argc == 3 && !strcmp( argv[1], "--decode" ) )
    {
        printf( "Decoded: %s\n", qPrintable( Utils::decodeIMAPutf7( argv[2] )));
        return 1;
    }

    // If system tray is not yet available, wait up to 60 seconds
    int passed = 0;

    while ( true )
    {
        if ( QSystemTrayIcon::isSystemTrayAvailable() )
            break;

        if ( passed == 0 )
            qDebug("Waiting for system tray to become available");

        passed++;

        if ( passed > 120 )
            Utils::fatal("Sorry, system tray cannot be controlled "
                         "through this addon on your operating system");

        QThread::usleep( 500 );
    }


    // This prevents exiting the application when the dialogs are closed on Gnome/XFCE
    a.setQuitOnLastWindowClosed( false );

    // Set data for QSettings
    QCoreApplication::setOrganizationName("ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("birdtray");

    // Load settings
    pSettings = new Settings();
    pSettings->load();

    if ( argc == 2 && !strcmp( argv[1], "--debug" ) )
        pSettings->mVerboseOutput = true;

    TrayIcon trayicon;
    return a.exec();
}
