#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryFile>

#include "setting_newemail.h"
#include "dialogaddeditnewemail.h"


Setting_NewEmail::Setting_NewEmail()
{

}

Setting_NewEmail Setting_NewEmail::fromJSON(const QJsonObject &obj)
{
    Setting_NewEmail newmail;

    if ( obj.contains( "to" ) )
        newmail.mRecipient = obj["to"].toString();

    if ( obj.contains( "subject" ) )
        newmail.mSubject = obj["subject"].toString();

    if ( obj.contains( "message" ) )
        newmail.mMessage = QByteArray::fromBase64( obj["message"].toString().toUtf8() );

    newmail.mName = obj["name"].toString();
    return newmail;
}

QJsonObject Setting_NewEmail::toJSON() const
{
    // We use it this way to easily extend this in future while keeping current settings
    QJsonObject obj;

    if ( !mRecipient.isEmpty() )
        obj["to"] = mRecipient;

    if ( !mSubject.isEmpty() )
        obj["subject"] = mSubject;

    if ( !mMessage.isEmpty() )
        obj["message"] = QString::fromLatin1( mMessage.toUtf8().toBase64() );

    obj["name"] = mName;

    return obj;
}

Setting_NewEmail Setting_NewEmail::fromByteArray(const QByteArray &str)
{
    QJsonDocument doc = QJsonDocument::fromBinaryData( str );

    if ( !doc.isObject() )
        return Setting_NewEmail();

    return fromJSON( doc.object() );
}

bool Setting_NewEmail::edit()
{
    DialogAddEditNewEmail dlg;

    dlg.leMenuEntry->setText( mName );
    dlg.leMessage->setText( mMessage );
    dlg.leSubject->setText( mSubject );
    dlg.leTo->setText( mRecipient );

    if ( dlg.exec() == QDialog::Accepted )
    {
        mName = dlg.leMenuEntry->text();
        mMessage = dlg.leMessage->toPlainText();
        mSubject = dlg.leSubject->text();
        mRecipient = dlg.leTo->text();

        return true;
    }

    return false;
}

QString Setting_NewEmail::menuentry() const
{
    return mName;
}

QString Setting_NewEmail::asArgs() {
    QStringList args;
    if (!mRecipient.isEmpty()) {
        args.push_back("to='" + mRecipient.replace('\'', "\u2032") + "'");
    }
    if (!mSubject.isEmpty()) {
        args.push_back("subject='" + mSubject.replace('\'', "\u2032") + "'");
    }
    if (!mMessage.isEmpty()) {
        args.push_back("body='" + mMessage.replace('\'', "\u2032") + "'");
    }
    return args.join(",");
}
