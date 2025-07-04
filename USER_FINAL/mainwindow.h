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
#include <QRadioButton>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QLabel>
#include <QScrollBar>
#include <QMessageBox>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

void calcThisAndNextWeek(QDate &outStartOfWeek, QDate &outEndOfNextWeek);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    //페이지넘기는 함수들
    void go_to_main();
    void go_to_chat();
    void go_to_check();
    void go_to_pt();
    void go_to_info();
    void go_to_login();
    void go_to_logout();
    //
    // db연결
    void db_open();
    void query_clicked();
    void user_login();

    void pt_possible();
    void user_info1();
    QString input_id ;
    QString input_password ;
    QVector <int> pt_id;

    void pt_schedule();
    void user_info2();
    QString dateString;
    void pt_send();

    void show_main_btn();
    void update_possible_day(const QDate &date);
    void send_click_time();
    void change_pt_daytime();
    void change_pt_daytime_real();
    int send_pt_id = 0;
    QString timeStr = QDateTime::currentDateTime().toString("MM-dd HH:mm");


    struct user_info
    {
        int status_num;
        QString pt_time;
        QString pt_day;
        QString input_id_num;
        QString U_PHONENUM;
        QString U_TRAINER;
        QString U_PT_CNT;
        QString U_TAINER_NAME_NUM;
        QString U_TRAINER_NAME;
        QString U_PT_TIME;
        QString dateString;
        int selected_Button_Index;
    }; //유저정보 구조체




    QString statusToString(int status_num) {
        switch (status_num) {
        case 0: return "완료";
        case 1: return "대기";
        case 2: return "거절";
        case 3: return "예약"; //승인
        case 4: return "변경 대기";
        case 5: "변경 완료 예약";
        case 6: "변경 거절";
        default:   return "알 수 없음";
        }
    }


    ~MainWindow();


private slots:

    void onPushButtonClicked();
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError);

    // 아래 6개 슬롯은, 각 체크박스가 토글될 때 호출되어 DB UPDATE 수행
    void on_checkBox_exe1_toggled(bool checked);
    void on_checkBox_exe2_toggled(bool checked);
    void on_checkBox_exe3_toggled(bool checked);
    void on_checkBox_exe4_toggled(bool checked);
    void on_checkBox_exe5_toggled(bool checked);
    void on_checkBox_exe6_toggled(bool checked);

    void show_my_exercise(const QDate &selectedDate);
    void pt_click_num(const QDate &selectedDate);
    void any_btn_clicked();

private:

    bool signalReceived = false;

    Ui::MainWindow *ui;
    QTcpServer     *server;
    QList<QTcpSocket*> clients;
    QTcpSocket *socket;


    user_info user_str;

    QDate currentDate;
    void appendMessage(QVBoxLayout *layout, const QString &text);
    user_info u; //다른 함수에서 저장한 구조체 값을 사용 하기 위해 class 멤버 추가
};


#endif // MAINWINDOW_H
