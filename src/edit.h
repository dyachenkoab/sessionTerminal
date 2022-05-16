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
    bool cameOutside = false;

public:
    Edit( QWidget *parent = nullptr );
    ~Edit() = default;
    void setHandler( DBusHandler *value );
    void sendHtml() const;

protected:
    void dropEvent(QDropEvent *e) override;

private:
    QVariantList prepareCharInfo() const;
    void cursorChanged() const;
    void textChange();

};

#endif // EDIT_H
