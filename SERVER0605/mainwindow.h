#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QVBoxLayout>
#include <QDateTime>

#include "pt_change.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void db_open();
    void turn_page_chat();
    void turn_page_pt();
    QString choice_option ="";
    QList<pt_change*> pt_change_list;
    // void show_pt_list();
    QString timeStr = QDateTime::currentDateTime().toString("MM-dd HH:mm");


    ~MainWindow();

private slots:
    void onPushButtonClicked();
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void show_pt_list();
    // ✨ 새 슬롯 추가! ✨
    // pt_change 위젯이 finished 시그널을 보낼 때 호출될 함수
    void handlePtChangeFinished(pt_change* widget_to_delete);

private:
    Ui::MainWindow *ui;
    QTcpServer     *server;
    QList<QTcpSocket*> clients;

    QMap<QTcpSocket*, QString> socketMap;  // 소켓 → ID
    QMap<QString, QTcpSocket*> clientMap;  // ID → 소켓

    void appendMessage(QVBoxLayout *layout, const QString &text);
};

#endif // MAINWINDOW_H
