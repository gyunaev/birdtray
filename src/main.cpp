#include <QApplication>

#include "dialogsettings.h"
#include "trayicon.h"
#include "settings.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if ( !QSystemTrayIcon::isSystemTrayAvailable() )
        qFatal( "Sorry, system tray cannot be controlled through this addon on your operating system");

    // Set data for QSettings
    QCoreApplication::setOrganizationName("ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("birdtray");

    // Load settings
    pSettings = new Settings();
    pSettings->load();

    TrayIcon trayicon;
    trayicon.show();

    return a.exec();
}
