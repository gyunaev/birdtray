#ifndef BIRDTRAY_EVENT_FILTER_H
#define BIRDTRAY_EVENT_FILTER_H


#include <QtCore/QByteArray>
#include <QtCore/QAbstractNativeEventFilter>

/**
 * Filter to handle WM_CLOSE events on Windows.
 */
class BirdtrayEventFilter: public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
};


#endif // BIRDTRAY_EVENT_FILTER_H
