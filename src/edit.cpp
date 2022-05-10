#include "edit.h"

void Edit::setHandler( DBusHandler *value )
{
    if ( !m_handler )
        m_handler = value;
}

Edit::Edit( QWidget *parent ) : QTextEdit( parent )
{
}

// inner events handlers
void Edit::keyPressed( const QString &key ) const
{
    QVariantList arg = prepareCharInfo( key );
    m_handler->sendMessageWithID( arg, "keyPress" );
}

void Edit::allSelected() const
{
    m_handler->sendMessageWithID( "selectAll" );
}

void Edit::cursorChanged() const
{
    m_handler->sendMessageWithID( textCursor().position(), "cursorPosition" );
}

void Edit::charRemoved() const
{
    m_handler->sendMessageWithID( "removeChar" );
}

void Edit::charDeleted() const
{
    m_handler->sendMessageWithID( "deleteChar" );
}

//---------------------------------------------------

QVariantList Edit::prepareCharInfo( const QString &key ) const
{
    QTextCharFormat format( textCursor().charFormat() );
    CharInfo charInfo;
    charInfo.key	    = key;
    charInfo.color	    = format.foreground().color().name();
    charInfo.fontString = format.font().toString();
    return QVariantList() << QVariant::fromValue( charInfo );
}

void Edit::changeSelection() const
{
    QTextCursor cursor( textCursor() );
    if ( cursor.hasSelection() ) {
        m_handler->sendMessageWithID( cursor.selectionStart(), cursor.selectionEnd(),
                        "setSelection" );
    }
}

void Edit::mouseReleaseEvent( QMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        cursorChanged();
    }
    changeSelection();
}

void Edit::insertFromMimeData( const QMimeData *data )
{
    //Узкое место, нужна система с управлением разделённой памятью
    if ( data->hasHtml() ) {
        insertHtml( data->html() );
        m_handler->sendMessageWithID( data->html(), "insertHtml" );
    } else if ( data->hasText() ) {
        insertPlainText( data->text() );
        m_handler->sendMessageWithID( data->text(), "insertText" );
    }
}

void Edit::keyReleaseEvent( QKeyEvent *event )
{
    QString key = event->text();
    if ( key.size() && ( key.end() - 1 )->isPrint() ) {
        keyPressed( key );
        return;
    }

    if ( event->matches( QKeySequence::SelectAll ) ) {
        allSelected();
        return;
    }

    if ( event->modifiers() == Qt::ShiftModifier ) {
        if ( event->matches( QKeySequence::SelectNextChar ) ||
             event->matches( QKeySequence::SelectPreviousChar ) ||
             event->matches( QKeySequence::SelectNextLine ) ||
             event->matches( QKeySequence::SelectPreviousLine ) ) {
            changeSelection();
            return;
        }
    }

    switch ( event->key() ) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            cursorChanged();
            break;
        case Qt::Key_Return:
            keyPressed( "\n" );
            break;
        case Qt::Key_Tab:
            keyPressed( "\t" );
            break;
        case Qt::Key_Backspace:
            charRemoved();
            break;
        case Qt::Key_Delete:
            charDeleted();
            break;
        default:
#ifdef QT_DEBUG
            qInfo() << "unknown event";
#endif
            break;
    }
}
