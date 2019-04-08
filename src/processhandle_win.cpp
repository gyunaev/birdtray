#include <utility>
#include <windows.h>
#include <tlhelp32.h>

#include "processhandle_win.h"
#include "utils.h"

#define WAIT_TIME_SECONDS 100


ProcessHandle_Win::ProcessHandle_Win(const QString &executablePath) :
        ProcessHandle(executablePath) {
}

ProcessHandle_Win::~ProcessHandle_Win() {
    if (processWaiter != nullptr) {
        processWaiter->requestInterruption();
    }
    if (processHandle != nullptr) {
        CloseHandle(processHandle);
    }
}

AttachResult ProcessHandle_Win::attach() {
    if (ProcessHandle::attach() == SUCCESS || isAttached()) {
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

HANDLE ProcessHandle_Win::findProcess() {
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

bool ProcessHandle_Win::isAttached() {
    return processHandle != nullptr && processWaiter != nullptr && processWaiter->isRunning();
}

void ProcessHandle_Win::ProcessWaiter::run() {
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
