#ifndef BIRDTRAY_BIRDTRAY_APP_H
#define BIRDTRAY_BIRDTRAY_APP_H

#include <QApplication>
#include <QTranslator>
#include <QtCore/QCommandLineParser>
#include "settings.h"
#include "autoupdater.h"
#include "trayicon.h"


/**
 * Represents the Birdtray application.
 */
class BirdtrayApp: public QApplication {
    Q_OBJECT;
public:
    
    /**
     * Creates a new Birdtray application.
     *
     * @param argc The number of command line arguments.
     * @param argv The command line arguments.
     */
    BirdtrayApp(int &argc, char** argv);
    
    ~BirdtrayApp() override;
    
    /**
     * @return The main Birdtray app instance.
     */
    static BirdtrayApp* get();
    
    /**
     * @return The Birdtray settings.
     */
    Settings* getSettings() const;
    
    /**
     * @return The auto updater instance.
     */
    AutoUpdater* getAutoUpdater() const;
    
    /**
     * @return The tray icon.
     */
    TrayIcon* getTrayIcon() const;

protected:
    bool event(QEvent* event) override;

private:
    /**
     * A translator holding the current Qt system translation
     */
    QTranslator qtTranslator;
    
    /**
     * A translator holding the current translation for dynamic (not hardcoded) strings.
     */
    QTranslator dynamicTranslator;
    
    /**
     * A translator holding the current translation for the UI.
     */
    QTranslator mainTranslator;
    
    /**
     * The Birdtray settings.
     */
    Settings* settings = nullptr;
    
    /**
     * An auto updater.
     */
    AutoUpdater* autoUpdater = nullptr;
    
    /**
     * The system tray icon.
     */
    TrayIcon* trayIcon = nullptr;
    
    /**
     * Load the translations for the current system locale.
     *
     * @return True, if a translation for the current locale has been loaded successfully.
     */
    bool loadTranslations();
    
    /**
     * Try to load the translation.
     *
     * @param translator The translator to load the translation
     * @param locale The locale of the translation to load.
     * @param translationName The name of the translation to load.
     * @param paths A list of directories to search for the translation to load.
     * @return True, if the translation was loaded successfully.
     */
    static bool loadTranslation(QTranslator &translator, QLocale &locale,
                                const QString &translationName, const QStringList &paths);
    
    /**
     * Parse the command line arguments.
     *
     * @param parser A command line parser that will contain the parsed arguments.
     */
    void parseCmdArguments(QCommandLineParser &parser);
    
    /**
     * Wait for the system tray to become available and exit if it doesn't within 60 seconds.
     */
    static void ensureSystemTrayAvailable();
};


#endif /* BIRDTRAY_BIRDTRAY_APP_H */
