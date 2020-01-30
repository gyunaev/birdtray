#include <QApplication>
#include "birdtrayapp.h"
#ifdef Q_OS_WIN
#  include <windows.h>
#  include <cstdio>
#  include <cstring>
#endif /* Q_OS_WIN  */

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN
    for (int i = 1; i < argc; i++) {
        if (strcmp("--debug", argv[i]) == 0) {
            AttachConsole(ATTACH_PARENT_PROCESS);
            // reopen the std I/O streams to redirect I/O to the parents console.
            freopen("CON", "w", stdout);
            freopen("CON", "w", stderr);
            freopen("CON", "r", stdin);
            break;
        }
    }
#endif /* Q_OS_WIN */
    BirdtrayApp app(argc, argv);
    return QApplication::exec();
}
