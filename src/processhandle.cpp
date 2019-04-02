#include <utility>
#include <QtCore/QFileInfo>

#include "processhandle.h"
#include "utils.h"
#ifdef Q_OS_WIN
#  include "processhandle_win.h"
#endif


static int registerExitReasonMetaType() Q_DECL_NOTHROW {
    try {
        return qRegisterMetaType<ProcessHandle::ExitReason>("ExitReason");
    } catch (...) {
        Utils::fatal("Failed to register ExitReason meta type.");
    }
}

Q_DECL_UNUSED const int ProcessHandle::ExitReason::_typeId = registerExitReasonMetaType();


ProcessHandle::ProcessHandle(QString executablePath):
        executablePath(std::move(executablePath)) {
}

ProcessHandle::~ProcessHandle() {
    if (process != nullptr) {
        process->deleteLater();
    }
}

ProcessHandle* ProcessHandle::create(const QString& executablePath) {
#ifdef Q_OS_WIN
    return new ProcessHandle_Win(executablePath);
#else
    return new ProcessHandle(std::move(executablePath));
#endif
}

QString& ProcessHandle::getExecutablePath() {
    return executablePath;
}

void ProcessHandle::attachOrStart() {
    AttachResult attachResult = attach();
    if (attachResult == AttachResult::PROCESS_NOT_RUNNING) {
        start();
        return;
    }
    if (attachResult == AttachResult::SYSTEM_ERROR) {
        emit finished(ExitReason(true, tr("Failed to attach to running instance of %1").arg(
                getExecutableName())));
    }
}

AttachResult ProcessHandle::attach() {
    return process == nullptr ? PROCESS_NOT_RUNNING : SUCCESS;
}

void ProcessHandle::onProcessError(QProcess::ProcessError) {
    QProcess* erroredProcess = process;
    emit finished(ExitReason(true, erroredProcess->errorString()));
    process->deleteLater();
    process = nullptr;
    
}

void ProcessHandle::onProcessFinished(int, QProcess::ExitStatus) {
    emit finished(ExitReason());
    process->deleteLater();
    process = nullptr;
}

void ProcessHandle::start() {
    QProcess* oldProcess = process;
    if (oldProcess != nullptr) {
        oldProcess->blockSignals(true);
        oldProcess->deleteLater();
    }
    QProcess* newProcess = process = new QProcess();
    connect(newProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &ProcessHandle::onProcessFinished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(newProcess, &QProcess::errorOccurred, this, &ProcessHandle::onProcessError);
#endif
    newProcess->start(getExecutablePath());
}

QString ProcessHandle::getExecutableName() {
    QString path = QFileInfo(executablePath).fileName();
    if (path.endsWith('"')) {
        path.chop(1);
    }
    return path.trimmed();
}
