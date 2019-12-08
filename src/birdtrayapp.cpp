#include <QtCore/QLibraryInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QSharedMemory>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

#include "birdtrayapp.h"
#ifdef Q_OS_WIN
#  include "birdtrayeventfilter.h"
#endif /* Q_OS_WIN */
#include "utils.h"
#include "morkparser.h"

// Maximum memory size for inter-application communication
static const int SHARED_MEMORY_SIZE = 4096;
static const char * SHARED_MEMORY_TOKEN = "aeheewah7kahDauteeCeeP0jeipheiJuShailona0heidahyee2chahfiiLoh1ah";


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

    parseCmdArguments( mParser );
    
    QString morkPath = mParser.value("dump-mork");
    if (!morkPath.isEmpty()) {
        exit(MorkParser::dumpMorkFile(morkPath));
        return;
    }
    QString imapString = mParser.value("decode");
    if (!imapString.isEmpty()) {
        printf("Decoded: %s\n", qPrintable(Utils::decodeIMAPutf7(imapString)));
        exit(EXIT_SUCCESS);
        return;
    }

    // Load settings
    settings = new Settings( mParser.isSet("debug"));

    if ( mParser.isSet("reset-settings")) {
        settings->save(); // Saving without loading will reset the values
    } else {
        settings->load();
    }

    if (!translationLoadedSuccessfully) {
        Utils::debug("Failed to load translation for %s", qPrintable(QLocale::system().name()));
    }
}

BirdtrayApp::~BirdtrayApp() {
    delete trayIcon;
    delete autoUpdater;
    delete settings;
}

bool BirdtrayApp::isInstanceRunning()
{
    QSharedMemory * sharedMemory = new QSharedMemory( SHARED_MEMORY_TOKEN );

    // If we can attach to it, the segment already exists
    if ( sharedMemory->attach() )
    {
        // Another instance exists; send the command-line there
        QJsonObject commands;

        // Add more commands here - don't forget to add handling for them in TrayIcon::checkSharedMemoryMessage()
        if ( mParser.isSet("toggle" ) )
            commands["toggle"] = 1;

        // Do we have any commands to send there?
        if ( !commands.isEmpty() )
        {
            QByteArray jsondata = QJsonDocument( commands ).toJson();

            if ( jsondata.size() < SHARED_MEMORY_SIZE - 2 )
            {
                // Write the size first, then the string
                if ( sharedMemory->lock() )
                {
                    char * data = (char*) sharedMemory->data();
                    *((short*)data) = jsondata.size();
                    memcpy( data + 2, jsondata.data(), jsondata.size() );

                    sharedMemory->unlock();
                }
                else
                    qFatal("Failed to lock shared memory segment");
            }
            else
            {
                qFatal("Argument list too long");
            }
        }
        else
            qWarning("Another instance of Birdtray is already running");

        // Clean up - this is important since we're finishing this instance
        delete sharedMemory;
        return true;
    }

    // Create a new segment - this is the first running instance
    if ( !sharedMemory->create( SHARED_MEMORY_SIZE ) )
    {
        QMessageBox::critical( 0,
                               tr("Shared memory segment failed"),
                               tr("Failed to create a shared memory segment: %1").arg( sharedMemory->errorString()) );
        return false;
    }

    // Reset it up so our checker knows there's no data yet
    *((short*) sharedMemory->data()) = 0;

    ensureSystemTrayAvailable();

#ifdef Q_OS_WIN
    BirdtrayEventFilter filter;
    installNativeEventFilter(&filter);
#endif /* Q_OS_WIN */

    autoUpdater = new AutoUpdater();
    trayIcon = new TrayIcon( mParser.isSet("settings"), sharedMemory );
    return false;
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
            { "toggle", tr("Toggle Thunderbird visibility (requires another running instance)")},
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
