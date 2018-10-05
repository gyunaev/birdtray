#include <QApplication>

#include "dialogsettings.h"
#include "trayicon.h"
#include "settings.h"
#include "morkparser.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if ( argc == 3 && !strcmp( argv[1], "--dumpmork" ) )
    {
        MorkParser::dumpMorkFile( argv[2] );
        return 1;
    }

    if ( !QSystemTrayIcon::isSystemTrayAvailable() )
        qFatal( "Sorry, system tray cannot be controlled through this addon on your operating system");

    // This prevents exiting the application when the dialogs are closed on Gnome/XFCE
    a.setQuitOnLastWindowClosed( false );

    // Set data for QSettings
    QCoreApplication::setOrganizationName("ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("birdtray");

    // Load settings
    pSettings = new Settings();
    pSettings->load();

    TrayIcon trayicon;
    return a.exec();
}
