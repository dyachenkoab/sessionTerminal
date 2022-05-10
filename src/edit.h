#ifndef EDIT_H
#define EDIT_H

#include "Structs.h"
#include "dbushandler.h"
#include <QBuffer>
#include <QDebug>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QObject>
#include <QTextEdit>

struct CharInfo;

class Edit : public QTextEdit
{
    Q_OBJECT

    DBusHandler *m_handler = nullptr;

public:
    Edit( QWidget *parent = nullptr );
    ~Edit(){ qInfo() << "edit stopped"; }// = default;
    void setHandler( DBusHandler *value );

private:
    QVariantList prepareCharInfo( const QString &key ) const;
    void keyPressed( const QString &key ) const;
    void allSelected() const;
    void cursorChanged() const;
    void charRemoved() const;
    void charDeleted() const;
    void changeSelection() const;

protected:
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void insertFromMimeData( const QMimeData *data ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
};

#endif // EDIT_H
