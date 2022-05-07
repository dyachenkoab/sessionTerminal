#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "mainwindow.h"

void parseCommandLine( QString &arg, bool &isolated, const QApplication &a );

int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );

    QString arg;
    bool isolated = false;

    parseCommandLine( arg, isolated, a );

    MainWindow w( arg, isolated );

    const QRect availableGeometry = QApplication::desktop()->availableGeometry( &w );
    w.resize( availableGeometry.width() / 2, ( availableGeometry.height() * 2 ) / 3 );
    w.move( ( availableGeometry.width() - w.width() ) / 2,
        ( availableGeometry.height() - w.height() ) / 2 );

    w.show();

    return a.exec();
}

void parseCommandLine( QString &arg, bool &isolated, const QApplication &a )
{
    QCommandLineParser parser;
    parser.setApplicationDescription( "One Session Terminal" );
    parser.addPositionalArgument(
        "name", QCoreApplication::translate( "main", "Private session name" ) );
    parser.addHelpOption();

    QCommandLineOption privateSessionOption(
        QStringList() << "s"
              << "session",
        QCoreApplication::translate( "main", "Creates private session terminal" ) );
    parser.addOption( privateSessionOption );

    QCommandLineOption singleTerminalOption(
        QStringList() << "n"
              << "noSession",
        QCoreApplication::translate( "main", "Creates isolated sesison terminal" ) );
    parser.addOption( singleTerminalOption );

    if ( parser.parse( QCoreApplication::arguments() ) ) {
        parser.process( a );

        QStringList lst = parser.positionalArguments();
        if ( !lst.isEmpty() ) {
            arg = lst.first();
        }
        isolated = parser.isSet( singleTerminalOption );
        return;
    }

    qInfo() << parser.errorText() << "skip";
}
