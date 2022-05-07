#include "dbushandler.h"
#include "edit.h"

QVariantList DBusHandler::getToolbarState() const
{
    return toolbarState;
}

DBusHandler::DBusHandler( const QString &id, const QString &privateSession, bool isolated,
              Edit *textEdit )
    : id( id ), isolated( isolated ), conn( new QDBusConnection( QDBusConnection::sessionBus() ) ),
      textEdit( textEdit ), sharedMemory( "SharedMemory" ), QObject( textEdit )
{
    setupDBusParameters( privateSession );
    registerClass();
    setupConnections();
    feedTextEditor();
}

DBusHandler::~DBusHandler()
{
    delete conn;
    delete iface;
}

//----------prepare-----------------
void DBusHandler::setupDBusParameters( const QString &privateSession )
{
    obj = "/test";

    i_face	= privateSession.isEmpty() ? "test.session" : ( "test." + privateSession );
    service = privateSession.isEmpty() ? ( "test.session._" + id )
                       : ( "test." + privateSession + "._" + id );

    if ( isolated ) {
        service = "test.isolated._" + id;
        i_face	= "test.isolated";
        ranged	= service;
    }
}

void DBusHandler::registerClass()
{
    if ( !conn->registerObject( obj, i_face, this, QDBusConnection::ExportAllSlots ) ) {
        fprintf( stderr, "%s\n",
             qPrintable( QDBusConnection::sessionBus().lastError().message() ) );
        exit( 0 );
    }

    if ( !conn->registerService( service ) ) {
        fprintf( stderr, "%s\n",
             qPrintable( QDBusConnection::sessionBus().lastError().message() ) );
        exit( 0 );
    }

    iface = new QDBusInterface( service, obj, i_face, QDBusConnection::sessionBus() );
    if ( !iface->isValid() ) {
        fprintf( stderr, "%s\n",
             qPrintable( QDBusConnection::sessionBus().lastError().message() ) );
        exit( 0 );
    }

    qRegisterMetaType<CharInfo>( "CharInfo" );
    qDBusRegisterMetaType<CharInfo>();
}

void DBusHandler::setupConnections()
{
    conn->connect( ranged, obj, i_face, "cursorPosition", this,
               SLOT( changeCursorPosition( QString, int ) ) );
    conn->connect( ranged, obj, i_face, "keyPress", this,
               SLOT( changeKey( QString, QVariantList ) ) );
    conn->connect( ranged, obj, i_face, "setSelection", this,
               SLOT( setSelection( QString, int, int ) ) );
    conn->connect( ranged, obj, i_face, "removeChar", this, SLOT( removeChar( QString ) ) );
    conn->connect( ranged, obj, i_face, "deleteChar", this, SLOT( deleteChar( QString ) ) );
    conn->connect( ranged, obj, i_face, "insertHtml", this,
               SLOT( htmlInsert( QString, QString ) ) );
    conn->connect( ranged, obj, i_face, "insertText", this,
               SLOT( textInsert( QString, QString ) ) );
    conn->connect( ranged, obj, i_face, "selectAll", this, SLOT( selectAll( QString ) ) );

    conn->connect( ranged, obj, i_face, "setBold", this, SLOT( bolded( bool ) ) );
    conn->connect( ranged, obj, i_face, "setUnderline", this, SLOT( underlined( bool ) ) );
    conn->connect( ranged, obj, i_face, "setItalic", this, SLOT( italiced( bool ) ) );
    conn->connect( ranged, obj, i_face, "textFamily", this, SLOT( textFamily( QString ) ) );
    conn->connect( ranged, obj, i_face, "textSize", this, SLOT( textSize( QString ) ) );
    conn->connect( ranged, obj, i_face, "textColor", this, SLOT( textColored( QString ) ) );
}

void DBusHandler::feedTextEditor()
{
    QString service = getServiceName();
    if ( !service.isEmpty() ) {
        if ( callFunction( service, "loadToSharedMemory" ).first().toBool() ) {
            loadFromMemory();
            callFunction( service, "detachSharedMemory" );
            QVariantList lst = callFunction( service, "getCharState" );
            if ( !lst.isEmpty() ) {
                toolbarState = lst;
                textColored( lst.at( COLOR ).toString() );
                changeCursorPosition( "0", lst.at( POSITION ).toInt() );
            }
        }
    }
}
//------------accept signals---------------

void DBusHandler::changeKey( const QString &id, const QVariantList &list )
{
    if ( id != this->id ) {
        CharInfo info = qdbus_cast<CharInfo>( list.at( 0 ) );
        QTextCursor cursor( textEdit->textCursor() );
        textEdit->insertPlainText( info.key );

        QFont font;
        font.fromString( info.fontString );

        QTextCharFormat format;
        QColor color( info.color );
        format.setFont( font );
        format.setForeground( color );

        cursor.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
        cursor.mergeCharFormat( format );
        textEdit->mergeCurrentCharFormat( format );
    }
}

void DBusHandler::selectAll( const QString &id )
{
    if ( id != this->id ) {
        QTextCursor cursor( textEdit->textCursor() );
        cursor.select( QTextCursor::Document );
        textEdit->setTextCursor( cursor );
    }
}

void DBusHandler::removeChar( const QString &id )
{
    if ( id != this->id ) {
        textEdit->textCursor().deletePreviousChar();
    }
}

void DBusHandler::deleteChar( const QString &id )
{
    if ( id != this->id ) {
        textEdit->textCursor().deleteChar();
    }
}

void DBusHandler::changeCursorPosition( const QString &id, const int &pos )
{
    if ( id != this->id ) {
        QTextCursor cursor( textEdit->textCursor() );
        cursor.clearSelection();
        cursor.setPosition( pos );
        textEdit->setTextCursor( cursor );
    }
}

void DBusHandler::setSelection( const QString &id, int start, int stop )
{
    if ( id != this->id ) {
        int delta = stop - start;
        QTextCursor cursor( textEdit->textCursor() );
        cursor.setPosition( stop );
        cursor.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor, delta );
        textEdit->setTextCursor( cursor );
    }
}

void DBusHandler::htmlInsert( const QString &id, const QString &html )
{
    if ( id != this->id ) {
        textEdit->insertHtml( html );
    }
}

void DBusHandler::textInsert( const QString &id, const QString &text )
{
    if ( id != this->id ) {
        textEdit->insertPlainText( text );
    }
}

void DBusHandler::bolded( bool bold )
{
    QTextCharFormat fmt;
    fmt.setFontWeight( bold ? QFont::Bold : QFont::Normal );
    mergeFormatOnWordOrSelection( fmt );
}

void DBusHandler::underlined( bool underline )
{
    QTextCharFormat fmt;
    fmt.setFontUnderline( underline );
    mergeFormatOnWordOrSelection( fmt );
}

void DBusHandler::italiced( bool italic )
{
    QTextCharFormat fmt;
    fmt.setFontItalic( italic );
    mergeFormatOnWordOrSelection( fmt );
}

void DBusHandler::textFamily( const QString &f )
{
    QTextCharFormat fmt;
    fmt.setFontFamily( f );
    mergeFormatOnWordOrSelection( fmt );
}

void DBusHandler::textSize( const QString &p )
{
    qreal pointSize = p.toFloat();
    if ( p.toFloat() > 0 ) {
        QTextCharFormat fmt;
        fmt.setFontPointSize( pointSize );
        mergeFormatOnWordOrSelection( fmt );
    }
}

void DBusHandler::textColored( const QString &c )
{
    QTextCharFormat fmt;
    QColor col( c );
    fmt.setForeground( col );
    mergeFormatOnWordOrSelection( fmt );
}

void DBusHandler::mergeFormatOnWordOrSelection( const QTextCharFormat &format )
{
    QTextCursor cursor( textEdit->textCursor() );
    if ( !cursor.hasSelection() )
        cursor.select( QTextCursor::WordUnderCursor );
    cursor.mergeCharFormat( format );
    textEdit->mergeCurrentCharFormat( format );
}

QVariantList DBusHandler::loadToSharedMemory()
{
    QBuffer buffer;
    buffer.open( QBuffer::ReadWrite );
    QDataStream out( &buffer );
    out << textEdit->toHtml();
    int size = buffer.size();

    if ( !sharedMemory.create( size ) ) {
        qInfo() << "can\'t load sharedmemory";
        return QVariantList() << false;
    }
    sharedMemory.lock();
    char *to	 = (char *)sharedMemory.data();
    const char *from = buffer.data().data();
    memcpy( to, from, qMin( sharedMemory.size(), size ) );
    sharedMemory.unlock();
    return QVariantList() << true;
}

void DBusHandler::loadFromMemory()
{
    if ( !sharedMemory.attach() ) {
        qInfo() << "Unable to attach to shared memory segment";
        return;
    }

    QBuffer buffer;
    QString data;
    QDataStream in( &buffer );

    sharedMemory.lock();
    buffer.setData( (char *)sharedMemory.constData(), sharedMemory.size() );
    buffer.open( QBuffer::ReadOnly );
    in >> data;
    sharedMemory.unlock();

    sharedMemory.detach();

    textEdit->setHtml( data );
}

QVariantList DBusHandler::detachSharedMemory()
{
    if ( sharedMemory.isAttached() ) {
        sharedMemory.detach();
        return QVariantList() << true;
    }
    return QVariantList() << false;
}

QVariantList DBusHandler::getCharState()
{
    QTextCharFormat format( textEdit->textCursor().charFormat() );
    QVariantList lst;
    lst << QVariant( format.font().bold() );
    lst << QVariant( format.font().underline() );
    lst << QVariant( format.font().italic() );
    lst << QVariant( QString::number( format.font().pointSizeF() ) );
    lst << QVariant( format.font().toString() );
    lst << QVariant( format.foreground().color().name() );
    lst << QVariant( textEdit->textCursor().position() );
    return lst;
}
//-------------------------------------
QString DBusHandler::getServiceName() const
{
    if ( isolated )
        return QString();

    auto registeredNames = conn->interface()->registeredServiceNames().value();
    for ( auto &service : registeredNames ) {
        if ( service.startsWith( i_face ) ) {
            if ( service != this->service ) {
                return service;
            }
        }
    }
    return QString();
}

void DBusHandler::sendMessageWithID( const QString &signalName ) const
{
    QDBusConnection connection( QDBusConnection::sessionBus() );
    QDBusMessage msg = QDBusMessage::createSignal( obj, i_face, signalName );
    msg << id;
    connection.send( msg );
}

void DBusHandler::sendMessageWithID( int arg1, int arg2, const QString &signalName ) const
{
    QDBusMessage msg = QDBusMessage::createSignal( obj, i_face, signalName );
    msg << id << arg1 << arg2;
    conn->send( msg );
}

QVariantList DBusHandler::callFunction( const QString &serviceName,
                    const QString &functionName ) const
{
    QDBusInterface _iface( serviceName, obj, i_face, QDBusConnection::sessionBus() );
    QDBusReply<QVariantList> lst = _iface.call( functionName );
    return lst;
}
