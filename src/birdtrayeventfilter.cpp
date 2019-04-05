#include <windows.h>
#include <QApplication>
#include "birdtrayeventfilter.h"


bool BirdtrayEventFilter::nativeEventFilter(
        const QByteArray &eventType, void *message, long *result) {
    if (eventType == "windows_generic_MSG") {
        MSG* messageEvent = static_cast<MSG*>(message);
        if (messageEvent->message == WM_CLOSE) {
            QApplication::quit();
            if (result != nullptr) {
                *result = 0;
            }
            return true;
        }
    }
    return false;
}
