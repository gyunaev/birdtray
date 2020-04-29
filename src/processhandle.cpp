#include <utility>
#include <windows.h>
#include <tlhelp32.h>

#include <QtCore/QFileInfo>
#include "processhandle.h"
#include "utils.h"
#include "log.h"

#define WAIT_TIME_SECONDS 100


static int registerExitReasonMetaType() Q_DECL_NOTHROW {
    try {
        return qRegisterMetaType<ProcessHandle::ExitReason>("ExitReason");
    } catch (...) {
        // Don't translate the message, because it gets marked as vanished
        // by the lupdate tool on non-Windows platforms.
        Log::fatal("Failed to register ExitReason meta type.");
    }
}

Q_DECL_UNUSED const int ProcessHandle::ExitReason::_typeId = registerExitReasonMetaType();


ProcessHandle::ProcessHandle(QString executablePath) :
        executablePath(std::move(executablePath)) {
}

ProcessHandle::~ProcessHandle() {
    if (processWaiter != nullptr) {
        processWaiter->requestInterruption();
    }
    if (processHandle != nullptr) {
        CloseHandle(processHandle);
    }
}

ProcessHandle* ProcessHandle::create(const QString &executablePath) {
    return new ProcessHandle(executablePath);
}

QString ProcessHandle::getExecutableName() const {
    QString path = QFileInfo(executablePath).fileName();
    if (path.endsWith('"')) {
        path.chop(1);
    }
    return path.trimmed();
}

AttachResult ProcessHandle::attach() {
    if (isAttached()) {
        return SUCCESS;
    }
    if (processWaiter != nullptr) {
        processWaiter->requestInterruption();
    }
    if (processHandle != nullptr) {
        CloseHandle(processHandle);
        processHandle = nullptr;
    }
    processHandle = findProcess();
    if (processHandle == nullptr) {
        return PROCESS_NOT_RUNNING;
    }
    processWaiter = new ProcessWaiter(this);
    processWaiter->start();
    return SUCCESS;
}

HANDLE ProcessHandle::findProcess() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, static_cast<DWORD>(NULL));
    if (Process32First(snapshot, &entry) == TRUE) {
        QString exeName = getExecutableName();
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (exeName.compare(Utils::stdWToQString(entry.szExeFile), Qt::CaseInsensitive) == 0) {
                CloseHandle(snapshot);
                return OpenProcess(SYNCHRONIZE, FALSE, entry.th32ProcessID);
            }
        }
    }
    CloseHandle(snapshot);
    return nullptr;
}

bool ProcessHandle::isAttached() {
    return processHandle != nullptr && processWaiter != nullptr && processWaiter->isRunning();
}

void ProcessHandle::ProcessWaiter::run() {
    DWORD waitResult;
    while ((waitResult = WaitForSingleObject(
            processHandle->processHandle, WAIT_TIME_SECONDS * 1000)) == WAIT_TIMEOUT) {
        if (this->isInterruptionRequested() || processHandle->processHandle == nullptr) {
            return;
        }
    }
    QString errorMsg = "";
    if (waitResult == WAIT_FAILED) {
        LPTSTR errorMsgBuffer;
        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPTSTR>(&errorMsgBuffer), 0, nullptr);
#ifdef UNICODE
        errorMsg = Utils::stdWToQString(errorMsgBuffer);
#else
        errorMsg = errorMsgBuffer;
#endif /* UNICODE */
        LocalFree(errorMsgBuffer);
    }
    emit processHandle->finished(ProcessHandle::ExitReason(waitResult == WAIT_FAILED, errorMsg));
    this->deleteLater();
    if (processHandle->processWaiter == this) {
        processHandle->processWaiter = nullptr;
        if (processHandle->processHandle != nullptr) {
            CloseHandle(processHandle->processHandle);
            processHandle->processHandle = nullptr;
        }
    }
}
