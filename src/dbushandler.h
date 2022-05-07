#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include "Structs.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDebug>
#include <QMetaType>
#include <QSharedMemory>
#include <QTextEdit>

class Edit;
class DBusHandler : public QObject
{
    Q_OBJECT
    QDBusConnection *conn;
    QDBusInterface *iface;
    QStringList names;

    Edit *textEdit;
    QSharedMemory sharedMemory;

    const QString id;
    const bool isolated;

    QVariantList toolbarState;

    QString obj;
    QString i_face;
    QString service;
    QString ranged;

public:
    DBusHandler( const QString &id, const QString &privateSession, bool isolated,
             Edit *textEdit );
    ~DBusHandler();

    QVariantList getToolbarState() const;

    QVariantList callFunction( const QString &serviceName, const QString &functionName ) const;
    void sendMessageWithID( const QString &signalName ) const;
    void sendMessageWithID( int arg1, int arg2, const QString &signalName ) const;

    template <typename T>
    void sendMessageWithID( const T &args, const QString &signalName ) const
    {
        QDBusMessage msg = QDBusMessage::createSignal( obj, i_face, signalName );
        msg << id << args;
        conn->send( msg );
    }

    template <typename T> void sendMessage( const T &args, const QString &signalName ) const
    {
        QDBusMessage msg = QDBusMessage::createSignal( obj, i_face, signalName );
        msg << args;
        conn->send( msg );
    }

public slots:
    void changeKey( const QString &id, const QVariantList &list );
    void selectAll( const QString &id );
    void removeChar( const QString &id );
    void deleteChar( const QString &id );
    void changeCursorPosition( const QString &id, const int &pos );
    void setSelection( const QString &id, int start, int stop );

    void htmlInsert( const QString &id, const QString &html );
    void textInsert( const QString &id, const QString &text );

    void bolded( bool bold );
    void underlined( bool underline );
    void italiced( bool italic );
    void textFamily( const QString &f );
    void textSize( const QString &p );
    void textColored( const QString &c );

    QVariantList loadToSharedMemory();
    QVariantList detachSharedMemory();
    QVariantList getCharState();

private:
    QString getServiceName() const;
    void setupDBusParameters( const QString &privateSession );
    void registerClass();
    void setupConnections();
    void mergeFormatOnWordOrSelection( const QTextCharFormat &format );
    void feedTextEditor();
    void loadFromMemory();
};

#endif // DBUSHANDLER_H
