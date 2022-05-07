#include "mainwindow.h"

MainWindow::MainWindow( const QString &privateSession, bool isolated, QWidget *parent )
    : QMainWindow( parent )
{
    const int idSize = 30;
    id		 = QUuid::createUuid()
         .toString()
         .replace( QString( "-" ), QString( "" ) )
         .remove( 0, 1 )
         .left( idSize );

    setupTextActions();

    textEdit = new Edit( this );
    handler	 = new DBusHandler( id, privateSession, isolated, textEdit );
    textEdit->setHandler( handler );

    QObject::connect( textEdit, &QTextEdit::currentCharFormatChanged, this,
              &MainWindow::currentCharFormatChanged );

    setCentralWidget( textEdit );
    textEdit->setFocus();

    setToolbar();
}


void MainWindow::setupTextActions()
{
    QToolBar *tb = addToolBar( tr( "Format Actions" ) );
    QMenu *menu  = menuBar()->addMenu( tr( "F&ormat" ) );

    const QIcon boldIcon =
        QIcon::fromTheme( "format-text-bold", QIcon( ":/images/textbold.png" ) );
    actionTextBold = menu->addAction( boldIcon, tr( "&Bold" ), this, &MainWindow::textBold );
    actionTextBold->setShortcut( Qt::CTRL + Qt::Key_B );
    actionTextBold->setPriority( QAction::LowPriority );
    QFont bold;
    bold.setBold( true );
    actionTextBold->setFont( bold );
    tb->addAction( actionTextBold );
    actionTextBold->setCheckable( true );

    const QIcon underlineIcon =
        QIcon::fromTheme( "format-text-underline", QIcon( ":/images/textunder.png" ) );
    actionTextUnderline =
        menu->addAction( underlineIcon, tr( "&Underline" ), this, &MainWindow::textUnderline );
    actionTextUnderline->setShortcut( Qt::CTRL + Qt::Key_U );
    actionTextUnderline->setPriority( QAction::LowPriority );
    QFont underline;
    underline.setUnderline( true );
    actionTextUnderline->setFont( underline );
    tb->addAction( actionTextUnderline );
    actionTextUnderline->setCheckable( true );

    const QIcon italicIcon =
        QIcon::fromTheme( "format-text-italic", QIcon( ":/images/textitalic.png" ) );
    actionTextItalic =
        menu->addAction( italicIcon, tr( "&Italic" ), this, &MainWindow::textItalic );
    actionTextItalic->setPriority( QAction::LowPriority );
    actionTextItalic->setShortcut( Qt::CTRL + Qt::Key_I );
    QFont italic;
    italic.setItalic( true );
    actionTextItalic->setFont( italic );
    tb->addAction( actionTextItalic );
    actionTextItalic->setCheckable( true );

    menu->addSeparator();

    comboFont = new QFontComboBox( tb );
    tb->addWidget( comboFont );

    connect( comboFont, QOverload<const QString &>::of( &QComboBox::activated ), this,
         &MainWindow::textFamily );

    comboSize = new QComboBox( tb );
    comboSize->setObjectName( "comboSize" );
    tb->addWidget( comboSize );
    comboSize->setEditable( true );

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach ( int size, standardSizes )
        comboSize->addItem( QString::number( size ) );
    comboSize->setCurrentIndex( standardSizes.indexOf( QApplication::font().pointSize() ) );

    connect( comboSize, QOverload<const QString &>::of( &QComboBox::activated ), this,
         &MainWindow::textSize );

    QPixmap pix( 16, 16 );
    pix.fill( Qt::black );
    actionTextColor = menu->addAction( pix, tr( "&Color..." ), this, &MainWindow::textColor );
    tb->addAction( actionTextColor );
}

//-------------toolbar events-------------------------
void MainWindow::textBold() const
{
    handler->sendMessage( actionTextBold->isChecked(), "setBold" );
}

void MainWindow::textUnderline() const
{
    handler->sendMessage( actionTextUnderline->isChecked(), "setUnderline" );
}

void MainWindow::textItalic() const
{
    handler->sendMessage( actionTextItalic->isChecked(), "setItalic" );
}

void MainWindow::textFamily( const QString &f ) const
{
    handler->sendMessage( f, "textFamily" );
}

void MainWindow::textSize( const QString &p ) const
{
    handler->sendMessage( p, "textSize" );
}

void MainWindow::textColor()
{
    QColor col = QColorDialog::getColor( textEdit->textColor(), this );
    if ( !col.isValid() )
        return;
    col.name();
    handler->sendMessage( col.name(), "textColor" );
}
//--------------------------------------
void MainWindow::colorChanged( const QColor &c )
{
    QPixmap pix( 16, 16 );
    pix.fill( c );
    actionTextColor->setIcon( pix );
}

void MainWindow::currentCharFormatChanged( const QTextCharFormat &format )
{
    fontChanged( format.font() );
    colorChanged( format.foreground().color() );
}

void MainWindow::fontChanged( const QFont &f )
{
    comboFont->setCurrentIndex( comboFont->findText( QFontInfo( f ).family() ) );
    comboSize->setCurrentIndex( comboSize->findText( QString::number( f.pointSize() ) ) );
    actionTextBold->setChecked( f.bold() );
    actionTextItalic->setChecked( f.italic() );
    actionTextUnderline->setChecked( f.underline() );
}

void MainWindow::setToolbar()
{
    QVariantList state = handler->getToolbarState();
    if ( !state.isEmpty() ) {
        actionTextBold->setChecked( state.at( BOLD ).toBool() );
        actionTextUnderline->setChecked( state.at( UNDERLINE ).toBool() );
        actionTextItalic->setChecked( state.at( ITALIC ).toBool() );
        comboSize->setCurrentText( state.at( SIZE ).toString() );
        QFont font;
        font.fromString( state.at( FONT ).toString() );
        comboFont->setCurrentFont( font );
        QColor color( state.at( COLOR ).toString() );
        colorChanged( color );
    }
}
