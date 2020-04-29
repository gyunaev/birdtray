#include <QApplication>
#include "birdtrayapp.h"
#ifdef Q_OS_WIN
#  include <windows.h>
#  include <cstdio>
#endif /* Q_OS_WIN  */

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // reopen the std I/O streams to redirect I/O to the parents console.
        FILE* newFile = nullptr;
        freopen_s(&newFile, "CON", "w", stdout);
        freopen_s(&newFile, "CON", "w", stderr);
        freopen_s(&newFile, "CON", "r", stdin);
    }
#endif /* Q_OS_WIN */
    BirdtrayApp app(argc, argv);
    return QApplication::exec();
}
