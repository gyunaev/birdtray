#include <utility>

#ifndef PROCESS_HANDLE_H
#define PROCESS_HANDLE_H

#include <sys/types.h>
#include <QObject>
#include <QString>
#include <QtCore/QArgument>
#include <QtCore/QProcess>


/**
 * Indicates the result of an process attach attempt.
 */
enum AttachResult {
    /**
     * The process was successfully attached to.
     */
    SUCCESS,

    /**
     * There was not process to attach to.
     */
    PROCESS_NOT_RUNNING,

    /**
     * There was another error.
     */
    SYSTEM_ERROR
};


/**
 * A Handle to a progress, which might or might not be running.
 */
class ProcessHandle: public QObject {
    Q_OBJECT
public:
    
    /**
     * Contains information about the exit reason of a process.
     */
    class ExitReason {
    public:
        ExitReason(bool error, QString description):
                error(error), description(std::move(description)) {};
        
        /**
         * @return true if the process exited because of an error, false otherwise.
         */
        bool isError() { return error; };

        /**
         * @return The description of the error, that caused the process to exit, if there is one.
         */
        const QString& getErrorDescription() { return description; }

    private:
        bool error;
        QString description;
    };
    
    ~ProcessHandle() override;
    
    /**
     * Create a new handle to the process identified by the executable.
     * It is not necessary for the process to exist.
     * The process in neither started nor attached to by creating this object.
     *
     * @param executablePath The path to the executable of the process.
     */
    static ProcessHandle* create(QString executablePath);

    /**
     * @return The path to the executable of the process.
     */
    QString& getExecutablePath();

    /**
     * Attach to a running process or create a new one and attach to it.
     * If this function fails, the finished signal is triggered.
     */
    void attachOrStart();
    
    /**
     * Attach to a running process. This does nothing if the this handler
     * is already attached to the running process.
     *
     * @return An AttachResult to indicate the success or failure reason.
     */
    virtual AttachResult attach();

signals:
    /**
     * A signal that is triggered when the attached process exits or fails to start.
     *
     * @param exitReason The reason for the exit.
     */
    void finished(ExitReason exitReason);

protected:
    /**
     * Protected constructor.
     *
     * @param executablePath The path to the executable of the process.
     */
    explicit ProcessHandle(QString executablePath);
    
    /**
     * @return The name of the executable file, without the path.
     */
    QString getExecutableName();

private slots:
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    /**
     * Start a process from the executable.
     * If this function fails, the finished signal is triggered.
     */
    void start();
    
    /**
     * The path to the executable that identifies the process.
     */
    QString executablePath;
    
    /**
     * The process that we started.
     */
    QProcess* process = nullptr;
};


#endif // PROCESS_HANDLE_H
