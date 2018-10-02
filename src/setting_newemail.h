#ifndef SETTING_NEWEMAIL_H
#define SETTING_NEWEMAIL_H

#include <QString>

class Setting_NewEmail
{
    public:
        Setting_NewEmail();

        // Serialization
        static Setting_NewEmail fromByteArray( const QByteArray& str );
        QByteArray toByteArray() const;

        // Editing via edit dialog
        bool    edit();

        QString menuentry() const;

        // Convert into Thunderbird command line arguments
        QString asArgs();

    private:
        QString     mName;
        QString     mRecipient;
        QString     mSubject;
        QString     mMessage;
};

#endif // SETTING_NEWEMAIL_H
