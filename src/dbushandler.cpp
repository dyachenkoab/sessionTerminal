#include "dbushandler.h"
#include "edit.h"

QVariantList DBusHandler::getToolbarState() const
{
    return m_toolbarState;
}

DBusHandler::DBusHandler( const QString &id, const QString &privateSession, bool isolated,
              Edit *textEdit )
    : m_id( id ), m_isolated( isolated ), m_conn( new QDBusConnection( QDBusConnection::sessionBus() ) ),
      m_textEdit( textEdit ), m_sharedMemory( "SharedMemory" ), QObject( textEdit )
{
    setupDBusParameters( privateSession );
    registerClass();
    setupConnections();
    feedTextEditor();
}

DBusHandler::~DBusHandler()
{
}

//----------prepare-----------------
void DBusHandler::setupDBusParameters( const QString &privateSession )
{
    m_objName = "/test";

    m_ifaceName	= privateSession.isEmpty() ? "test.session" : ( "test." + privateSession );
    m_serviceName = privateSession.isEmpty() ? ( "test.session._" + m_id )
                       : ( "test." + privateSession + "._" + m_id );

    if ( m_isolated ) {
        m_serviceName = "test.isolated._" + m_id;
        m_ifaceName	= "test.isolated";
        m_rangedName	= m_serviceName;
    }
}

void DBusHandler::registerClass()
{
    if ( !m_conn->registerObject( m_objName, m_ifaceName, this, QDBusConnection::ExportAllSlots ) ) {
        fprintf( stderr, "%s\n",
             qPrintable( QDBusConnection::sessionBus().lastError().message() ) );
        exit( 0 );
    }

    if ( !m_conn->registerService( m_serviceName ) ) {
        fprintf( stderr, "%s\n",
             qPrintable( QDBusConnection::sessionBus().lastError().message() ) );
        exit( 0 );
    }

    m_iface.reset( new QDBusInterface( m_serviceName, m_objName, m_ifaceName, QDBusConnection::sessionBus() ) );
    if ( !m_iface->isValid() ) {
        fprintf( stderr, "%s\n",
             qPrintable( QDBusConnection::sessionBus().lastError().message() ) );
        exit( 0 );
    }

    qRegisterMetaType<CharInfo>( "CharInfo" );
    qDBusRegisterMetaType<CharInfo>();
}

void DBusHandler::setupConnections()
{
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "cursorPosition", this,
               SLOT( changeCursorPosition( QString, int ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "keyPress", this,
               SLOT( changeKey( QString, QVariantList ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "setSelection", this,
               SLOT( setSelection( QString, int, int ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "removeChar", this, SLOT( removeChar( QString ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "deleteChar", this, SLOT( deleteChar( QString ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "insertHtml", this,
               SLOT( htmlInsert( QString, QString ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "insertText", this,
               SLOT( textInsert( QString, QString ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "selectAll", this, SLOT( selectAll( QString ) ) );

    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "setBold", this, SLOT( bolded( bool ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "setUnderline", this, SLOT( underlined( bool ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "setItalic", this, SLOT( italiced( bool ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "textFamily", this, SLOT( textFamily( QString ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "textSize", this, SLOT( textSize( QString ) ) );
    m_conn->connect( m_rangedName, m_objName, m_ifaceName, "textColor", this, SLOT( textColored( QString ) ) );
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
                m_toolbarState = lst;
                textColored( lst.at( COLOR ).toString() );
                changeCursorPosition( "0", lst.at( POSITION ).toInt() );
            }
        }
    }
}
//------------accept signals---------------

void DBusHandler::changeKey( const QString &id, const QVariantList &list )
{
    if ( id != this->m_id ) {
        CharInfo info = qdbus_cast<CharInfo>( list.at( 0 ) );
        QTextCursor cursor( m_textEdit->textCursor() );
        m_textEdit->insertPlainText( info.key );

        QFont font;
        font.fromString( info.fontString );

        QTextCharFormat format;
        QColor color( info.color );
        format.setFont( font );
        format.setForeground( color );

        cursor.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
        cursor.mergeCharFormat( format );
        m_textEdit->mergeCurrentCharFormat( format );
    }
}

void DBusHandler::selectAll( const QString &id )
{
    if ( id != this->m_id ) {
        QTextCursor cursor( m_textEdit->textCursor() );
        cursor.select( QTextCursor::Document );
        m_textEdit->setTextCursor( cursor );
    }
}

void DBusHandler::removeChar( const QString &id )
{
    if ( id != this->m_id ) {
        m_textEdit->textCursor().deletePreviousChar();
    }
}

void DBusHandler::deleteChar( const QString &id )
{
    if ( id != this->m_id ) {
        m_textEdit->textCursor().deleteChar();
    }
}

void DBusHandler::changeCursorPosition( const QString &id, const int &pos )
{
    if ( id != this->m_id ) {
        QTextCursor cursor( m_textEdit->textCursor() );
        cursor.clearSelection();
        cursor.setPosition( pos );
        m_textEdit->setTextCursor( cursor );
    }
}

void DBusHandler::setSelection( const QString &id, int start, int stop )
{
    if ( id != this->m_id ) {
        int delta = stop - start;
        QTextCursor cursor( m_textEdit->textCursor() );
        cursor.setPosition( stop );
        cursor.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor, delta );
        m_textEdit->setTextCursor( cursor );
    }
}

void DBusHandler::htmlInsert( const QString &id, const QString &html )
{
    if ( id != this->m_id ) {
        m_textEdit->insertHtml( html );
    }
}

void DBusHandler::textInsert( const QString &id, const QString &text )
{
    if ( id != this->m_id ) {
        m_textEdit->insertPlainText( text );
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
    QTextCursor cursor( m_textEdit->textCursor() );
    if ( !cursor.hasSelection() )
        cursor.select( QTextCursor::WordUnderCursor );
    cursor.mergeCharFormat( format );
    m_textEdit->mergeCurrentCharFormat( format );
}

QVariantList DBusHandler::loadToSharedMemory()
{
    QBuffer buffer;
    buffer.open( QBuffer::ReadWrite );
    QDataStream out( &buffer );
    out << m_textEdit->toHtml();
    int size = buffer.size();

    if ( !m_sharedMemory.create( size ) ) {
        qInfo() << "can\'t load sharedmemory";
        return QVariantList() << false;
    }
    m_sharedMemory.lock();
    char *to	 = (char *)m_sharedMemory.data();
    const char *from = buffer.data().data();
    memcpy( to, from, qMin( m_sharedMemory.size(), size ) );
    m_sharedMemory.unlock();
    return QVariantList() << true;
}

void DBusHandler::loadFromMemory()
{
    if ( !m_sharedMemory.attach() ) {
        qInfo() << "Unable to attach to shared memory segment";
        return;
    }

    QBuffer buffer;
    QString data;
    QDataStream in( &buffer );

    m_sharedMemory.lock();
    buffer.setData( (char *)m_sharedMemory.constData(), m_sharedMemory.size() );
    buffer.open( QBuffer::ReadOnly );
    in >> data;
    m_sharedMemory.unlock();

    m_sharedMemory.detach();

    m_textEdit->setHtml( data );
}

QVariantList DBusHandler::detachSharedMemory()
{
    if ( m_sharedMemory.isAttached() ) {
        m_sharedMemory.detach();
        return QVariantList() << true;
    }
    return QVariantList() << false;
}

QVariantList DBusHandler::getCharState()
{
    QTextCharFormat format( m_textEdit->textCursor().charFormat() );
    QVariantList lst;
    lst << QVariant( format.font().bold() );
    lst << QVariant( format.font().underline() );
    lst << QVariant( format.font().italic() );
    lst << QVariant( QString::number( format.font().pointSizeF() ) );
    lst << QVariant( format.font().toString() );
    lst << QVariant( format.foreground().color().name() );
    lst << QVariant( m_textEdit->textCursor().position() );
    return lst;
}
//-------------------------------------
QString DBusHandler::getServiceName() const
{
    if ( m_isolated )
        return QString();

    auto registeredNames = m_conn->interface()->registeredServiceNames().value();
    for ( auto &service : registeredNames ) {
        if ( service.startsWith( m_ifaceName ) ) {
            if ( service != this->m_serviceName ) {
                return service;
            }
        }
    }
    return QString();
}

void DBusHandler::sendMessageWithID( const QString &signalName ) const
{
    QDBusConnection connection( QDBusConnection::sessionBus() );
    QDBusMessage msg = QDBusMessage::createSignal( m_objName, m_ifaceName, signalName );
    msg << m_id;
    connection.send( msg );
}

void DBusHandler::sendMessageWithID( int arg1, int arg2, const QString &signalName ) const
{
    QDBusMessage msg = QDBusMessage::createSignal( m_objName, m_ifaceName, signalName );
    msg << m_id << arg1 << arg2;
    m_conn->send( msg );
}

QVariantList DBusHandler::callFunction( const QString &serviceName,
                    const QString &functionName ) const
{
    QDBusInterface _iface( serviceName, m_objName, m_ifaceName, QDBusConnection::sessionBus() );
    QDBusReply<QVariantList> lst = _iface.call( functionName );
    return lst;
}
