#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>

#include <QTcpServer>
#include <QTcpSocket>

#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlError>

#include <QList>
#include <QBoxLayout>
#include <QDate>    // QDate
#include <QDebug>   // qDebug()
#include <QCheckBox> // QCheckBox
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QLabel>
#include <QScrollBar>
#include <QMessageBox>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void go_to_login();
    void go_to_logout();
    void trainer_login();

    void db_open();

    QString input_id ;
    QString input_password ;
    QString timeStr = QDateTime::currentDateTime().toString("MM-dd HH:mm");

    ~MainWindow();

private slots:
    void onPushButtonClicked();
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError);


private:
    Ui::MainWindow *ui;
    QTcpServer     *server;
    QList<QTcpSocket*> clients;
    QTcpSocket *socket;

    void appendMessage(QVBoxLayout *layout, const QString &text);

};
#endif // MAINWINDOW_H
