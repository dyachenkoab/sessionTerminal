#include "mainwindow.h"

MainWindow::MainWindow( const QString &privateSession, bool isolated, QWidget *parent )
    : QMainWindow( parent )
{
    const int idSize = 30;
    m_id		 = QUuid::createUuid()
         .toString()
         .replace( QString( "-" ), QString( "" ) )
         .remove( 0, 1 )
         .left( idSize );

    setupTextActions();

    m_textEdit = new Edit( this );
    m_handler	 = new DBusHandler( m_id, privateSession, isolated, m_textEdit );
    m_textEdit->setHandler( m_handler );

    QObject::connect( m_textEdit, &QTextEdit::currentCharFormatChanged, this,
              &MainWindow::currentCharFormatChanged );

    setCentralWidget( m_textEdit );
    m_textEdit->setFocus();

    setToolbar();
}


void MainWindow::setupTextActions()
{
    QToolBar *tb = addToolBar( tr( "Format Actions" ) );
    QMenu *menu  = menuBar()->addMenu( tr( "F&ormat" ) );

    const QIcon boldIcon =
        QIcon::fromTheme( "format-text-bold", QIcon( ":/images/textbold.png" ) );
    m_actionTextBold = menu->addAction( boldIcon, tr( "&Bold" ), this, &MainWindow::textBold );
    m_actionTextBold->setShortcut( Qt::CTRL + Qt::Key_B );
    m_actionTextBold->setPriority( QAction::LowPriority );
    QFont bold;
    bold.setBold( true );
    m_actionTextBold->setFont( bold );
    tb->addAction( m_actionTextBold );
    m_actionTextBold->setCheckable( true );

    const QIcon underlineIcon =
        QIcon::fromTheme( "format-text-underline", QIcon( ":/images/textunder.png" ) );
    m_actionTextUnderline =
        menu->addAction( underlineIcon, tr( "&Underline" ), this, &MainWindow::textUnderline );
    m_actionTextUnderline->setShortcut( Qt::CTRL + Qt::Key_U );
    m_actionTextUnderline->setPriority( QAction::LowPriority );
    QFont underline;
    underline.setUnderline( true );
    m_actionTextUnderline->setFont( underline );
    tb->addAction( m_actionTextUnderline );
    m_actionTextUnderline->setCheckable( true );

    const QIcon italicIcon =
        QIcon::fromTheme( "format-text-italic", QIcon( ":/images/textitalic.png" ) );
    m_actionTextItalic =
        menu->addAction( italicIcon, tr( "&Italic" ), this, &MainWindow::textItalic );
    m_actionTextItalic->setPriority( QAction::LowPriority );
    m_actionTextItalic->setShortcut( Qt::CTRL + Qt::Key_I );
    QFont italic;
    italic.setItalic( true );
    m_actionTextItalic->setFont( italic );
    tb->addAction( m_actionTextItalic );
    m_actionTextItalic->setCheckable( true );

    menu->addSeparator();

    m_comboFont = new QFontComboBox( tb );
    tb->addWidget( m_comboFont );
//    connect( comboFont, &QComboBox::textActivated, this, &MainWindow::textFamily );
    connect( m_comboFont, QOverload<const QString &>::of( &QComboBox::activated ), this,
         &MainWindow::textFamily );

    m_comboSize = new QComboBox( tb );
    m_comboSize->setObjectName( "comboSize" );
    tb->addWidget( m_comboSize );
    m_comboSize->setEditable( true );

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    foreach ( int size, standardSizes )
        m_comboSize->addItem( QString::number( size ) );
    m_comboSize->setCurrentIndex( standardSizes.indexOf( QApplication::font().pointSize() ) );

//    connect( comboSize, &QComboBox::textActivated, this, &MainWindow::textSize );
    connect( m_comboSize, QOverload<const QString &>::of( &QComboBox::activated ), this,
         &MainWindow::textSize );

    QPixmap pix( 16, 16 );
    pix.fill( Qt::black );
    m_actionTextColor = menu->addAction( pix, tr( "&Color..." ), this, &MainWindow::textColor );
    tb->addAction( m_actionTextColor );
}

//-------------toolbar events-------------------------
void MainWindow::textBold() const
{
    m_handler->sendMessage( m_actionTextBold->isChecked(), "setBold" );
}

void MainWindow::textUnderline() const
{
    m_handler->sendMessage( m_actionTextUnderline->isChecked(), "setUnderline" );
}

void MainWindow::textItalic() const
{
    m_handler->sendMessage( m_actionTextItalic->isChecked(), "setItalic" );
}

void MainWindow::textFamily( const QString &f ) const
{
    m_handler->sendMessage( f, "textFamily" );
}

void MainWindow::textSize( const QString &p ) const
{
    m_handler->sendMessage( p, "textSize" );
}

void MainWindow::textColor()
{
    QColor col = QColorDialog::getColor( m_textEdit->textColor(), this );
    if ( !col.isValid() )
        return;
    col.name();
    m_handler->sendMessage( col.name(), "textColor" );
}
//--------------------------------------
void MainWindow::colorChanged( const QColor &c )
{
    QPixmap pix( 16, 16 );
    pix.fill( c );
    m_actionTextColor->setIcon( pix );
}

void MainWindow::currentCharFormatChanged( const QTextCharFormat &format )
{
    fontChanged( format.font() );
    colorChanged( format.foreground().color() );
}

void MainWindow::fontChanged( const QFont &f )
{
    m_comboFont->setCurrentIndex( m_comboFont->findText( QFontInfo( f ).family() ) );
    m_comboSize->setCurrentIndex( m_comboSize->findText( QString::number( f.pointSize() ) ) );
    m_actionTextBold->setChecked( f.bold() );
    m_actionTextItalic->setChecked( f.italic() );
    m_actionTextUnderline->setChecked( f.underline() );
}

void MainWindow::setToolbar()
{
    QVariantList state = m_handler->getToolbarState();
    if ( !state.isEmpty() ) {
        m_actionTextBold->setChecked( state.at( BOLD ).toBool() );
        m_actionTextUnderline->setChecked( state.at( UNDERLINE ).toBool() );
        m_actionTextItalic->setChecked( state.at( ITALIC ).toBool() );
        m_comboSize->setCurrentText( state.at( SIZE ).toString() );
        QFont font;
        font.fromString( state.at( FONT ).toString() );
        m_comboFont->setCurrentFont( font );
        QColor color( state.at( COLOR ).toString() );
        colorChanged( color );
    }
}
