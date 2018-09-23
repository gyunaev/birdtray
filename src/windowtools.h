#ifndef WINDOWTOOLSBASE_H
#define WINDOWTOOLSBASE_H

#include <QObject>

class WindowTools : public QObject
{
    Q_OBJECT

    public:
        static WindowTools * create();

        // Looks up and remembers Thunderbird window handle. Returns true if found,
        // false if not found.
        virtual bool    lookup() = 0;

        // Shows/activates the window
        virtual bool    show() = 0;

        // Hides/closes the window (without closing the process)
        virtual bool    hide() = 0;

        // Is window hidden? true if yes
        virtual bool    isHidden() = 0;

        // Closes the application via WM_CLOSE or similar
        virtual bool    closeWindow() = 0;

    protected:
        WindowTools();
        virtual ~WindowTools();
};

#endif // WINDOWTOOLSBASE_H
