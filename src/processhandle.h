#ifndef PROCESS_HANDLE_H
#define PROCESS_HANDLE_H

#include <windows.h>
#include <utility>
#include <sys/types.h>
#include <QObject>
#include <QString>
#include <QThread>
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
class ProcessHandle : public QObject {
Q_OBJECT;
public:
    
    /**
     * Contains information about the exit reason of a process.
     */
    class ExitReason {
    public:
        explicit ExitReason(bool error = false, QString description = "") :
                error(error), description(std::move(description)) {
        };
        
        /**
         * @return true if the process exited because of an error, false otherwise.
         */
        bool isError() const {
            return error;
        };
        
        /**
         * @return The description of the error, that caused the process to exit, if there is one.
         */
        const QString &getErrorDescription() const {
            return description;
        }
    
    private:
        const bool error;
        const QString description;
        Q_DECL_UNUSED static const int _typeId;
    };
    
    ~ProcessHandle() override;
    
    /**
     * Create a new handle to the process identified by the executable.
     * It is not necessary for the process to exist.
     * The process in neither started nor attached to by creating this object.
     *
     * @param executablePath The path to the executable of the process.
     */
    static ProcessHandle* create(const QString &executablePath);
    
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
    void finished(const ExitReason &exitReason);

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
    QString getExecutableName() const;

private:
    
    /**
     * Thread that waits for a process to exit.
     */
    class ProcessWaiter : public QThread {
        friend ProcessHandle;
    public:
        ~ProcessWaiter() override {
            quit();
            requestInterruption();
            wait();
        };
    
    protected:
        void run() override;
    
    private:
        explicit ProcessWaiter(ProcessHandle* processHandle) :
                QThread(), processHandle(processHandle) {
        }
        
        ProcessHandle* processHandle = nullptr;
    };
    
    /**
     * @return Weather the handle is attached to a process.
     */
    bool isAttached();
    
    /**
     * Get a handle to the running process for this handle,
     * or nullptr if no such process is running.
     *
     * @return A handle to the running process or nullptr.
     */
    HANDLE findProcess();
    
    /**
     * A handle to the process.
     */
    HANDLE processHandle = nullptr;
    
    /**
     * A thread waiting for the process to exit.
     */
    QThread* processWaiter = nullptr;
    
    /**
     * The path to the executable that identifies the process.
     */
    const QString executablePath;
};

Q_DECLARE_METATYPE(ProcessHandle::ExitReason)


#endif // PROCESS_HANDLE_H
