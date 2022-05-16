#include "edit.h"

void Edit::setHandler( DBusHandler *value )
{
    if ( !m_handler )
        m_handler = value;
}

void Edit::sendHtml() const
{
    QVariantList arg = prepareCharInfo();
    if ( m_handler )
        m_handler->sendMessageWithID( arg, "textChange" );
}

Edit::Edit( QWidget *parent ) : QTextEdit( parent )
{
    QObject::connect( this, &QTextEdit::textChanged, this, &Edit::textChange );
}

void Edit::cursorChanged() const
{
    if ( m_handler )
        m_handler->sendMessageWithID( textCursor().position(), "cursorPosition" );
}

void Edit::textChange()
{
    if ( hasFocus() || cameOutside ) {
        cameOutside = false;
        sendHtml();
    }
}

void Edit::dropEvent( QDropEvent *e )
{
    QTextEdit::dropEvent( e );
    cameOutside = true;
    textChange();
}

//---------------------------------------------------

QVariantList Edit::prepareCharInfo() const
{
    CharInfo charInfo;
    charInfo.text = toHtml();
    charInfo.pos  = textCursor().position();
    return QVariantList() << QVariant::fromValue( charInfo );
}
