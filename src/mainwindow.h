#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "edit.h"
#include "dbushandler.h"
#include "Structs.h"
#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTextEdit>
#include <QToolBar>
#include <QUuid>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( const QString &privateSession, bool isolated, QWidget *parent = nullptr );
    ~MainWindow() = default;

private:
    Edit *m_textEdit = nullptr;
    DBusHandler *m_handler = nullptr;
    QString m_id;

    QAction *m_actionTextBold;
    QAction *m_actionTextUnderline;
    QAction *m_actionTextItalic;
    QAction *m_actionTextColor;
    QFontComboBox *m_comboFont;
    QComboBox *m_comboSize;

private:
    void setupTextActions();
    void textBold() const;
    void textUnderline() const;
    void textItalic() const;
    void textFamily( const QString &f ) const;
    void textSize( const QString &p ) const;
    void textColor();

    void currentCharFormatChanged( const QTextCharFormat &format );
    void fontChanged( const QFont &f );
    void colorChanged( const QColor &c );
    void setToolbar();
};
#endif // MAINWINDOW_H
