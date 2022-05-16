#ifndef STRUCTS_H
#define STRUCTS_H
#include <QDBusArgument>
#include <QDebug>

enum Params { BOLD, UNDERLINE, ITALIC, SIZE, FONT, COLOR, POSITION };

struct CharInfo {
    QString text;
    int pos = -1;

    CharInfo() = default;
    ~CharInfo()			  = default;
    CharInfo( const CharInfo &other ) = default;
    CharInfo &operator=( const CharInfo &other ) = default;

    friend QDBusArgument &operator<<( QDBusArgument &argument, const CharInfo &info )
    {
        argument.beginStructure();
        argument << info.text;
        argument << info.pos;
        argument.endStructure();
        return argument;
    }
    friend const QDBusArgument &operator>>( const QDBusArgument &argument, CharInfo &info )
    {
        argument.beginStructure();
        argument >> info.text;
        argument >> info.pos;
        argument.endStructure();
        return argument;
    }

    friend QDebug operator<<( QDebug dbg, const CharInfo &info )
    {
        dbg << "key:" << info.text << "position:" << info.pos;
        return dbg;
    }
};
Q_DECLARE_METATYPE( CharInfo )

#endif // STRUCTS_H
