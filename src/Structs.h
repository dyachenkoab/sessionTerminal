#ifndef STRUCTS_H
#define STRUCTS_H
#include <QDBusArgument>
#include <QDebug>

enum Params { BOLD, UNDERLINE, ITALIC, SIZE, FONT, COLOR, POSITION };

struct CharInfo {
    QString key;
    QString color;
    QString fontString;

    CharInfo() {}
    CharInfo( QString key, QString color, QString fontString )
        : key( key ), color( color ), fontString( fontString )
    {
    }
    ~CharInfo()			  = default;
    CharInfo( const CharInfo &other ) = default;
    CharInfo &operator=( const CharInfo &other ) = default;

    friend QDBusArgument &operator<<( QDBusArgument &argument, const CharInfo &info )
    {
        argument.beginStructure();
        argument << info.key;
        argument << info.color;
        argument << info.fontString;
        argument.endStructure();
        return argument;
    }
    friend const QDBusArgument &operator>>( const QDBusArgument &argument, CharInfo &info )
    {
        argument.beginStructure();
        argument >> info.key;
        argument >> info.color;
        argument >> info.fontString;
        argument.endStructure();
        return argument;
    }

    friend QDebug operator<<( QDebug dbg, const CharInfo &info )
    {
        dbg << "key:" << info.key << "color:" << info.color
            << "fontString:" << info.fontString;
        return dbg;
    }
};
Q_DECLARE_METATYPE( CharInfo )

#endif // STRUCTS_H
