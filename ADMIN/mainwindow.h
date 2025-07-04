#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QStringList>
#include <QtSql/qsqldatabase.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QList>
#include <QDebug>
#include <QTime>
#include <vector>
#include <QDateTime>
#include <userbtn.h>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct std_todo{
    std::vector<int> user_id;        //유저 데이터
    std::vector<QDateTime> day;            //운동날짜
    std::vector<QString> EXERCISE1;        //운동숙제1
    std::vector<QString> EXERCISE2;        //운동숙제2
    std::vector<QString> EXERCISE3;        //운동숙제3
    std::vector<QString> EXERCISE4;        //운동숙제4
    std::vector<QString> EXERCISE5;        //운동숙제5
    std::vector<QString> EXERCISE6;        //운동숙제6
    bool yes_no1;    //운동했는지 안했는지
    bool yes_no2;    //운동했는지 안했는지
    bool yes_no3;    //운동했는지 안했는지
    bool yes_no4;    //운동했는지 안했는지
    bool yes_no5;    //운동했는지 안했는지
    bool yes_no6;    //운동했는지 안했는지
};

struct std_user_info{       //유저 전체정보 확인
    int user_id;
    QString name;
    QString birth;
    QString phonenum;
    int trainer_id;
    int user_pt_cnt;
};

struct db_pt_data{      //pt일정담는 구조체
    int pt_user_id;
    int pt_tr_id;
    QDateTime pt_day;       //원본 시간 저장
    QString pt_day_str;     //출력용으로 문자열 저장
    int status;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QString trainer_name;
    QString select_time; //캘린더에서 선택한 날짜
    QString dateString;
    QDate selectedDate;

    //스택페이지 이동함수
    void go_to_page_main();
    void go_to_page_clnt();
    void go_to_page_pt();
    void go_to_page_check();
    void go_to_page_close();
    void go_to_page_userin();
    //DB관련 함수
    void setDB();       //db접속설정
    void read_db();     //db열기
    void routine_write_db(int user_id);    //운동루틴 입력하는거
    void routine_road_db();    //운동루틴 불러오기

    void trainer_info();    //트레이너 이름 출력 함수
    void trainer_user();    //트레이너의 회원들 출력함수
    void get_user_info();   //유저정보 구조체에 로드하기
    //기능
    void set_user_info_Btn();       //회원정보 확인/수정 함수
    void deadline_db();             //마감버튼 클릭하면 마감하는거
    void clear_lineEdits();         // lineEdits 지우기
    void clear_user_name_Btn();     // 유저이름 버튼 지우기
    void set_user_name_Btn();       //유저이름 버튼 세팅
    void trBtn_handling();    //트레이너 선택 버튼 핸들링
    void new_user_insert();         //새로운 유저 등록 저장

    // MainWindow 클래스 안에 멤버 추가
    QPushButton* currentActiveUserBtn = nullptr;  // 사용자 버튼 클릭 상태 기억
    QPushButton* currentActiveTrainerBtn = nullptr;//트레이너 버튼 클릭 상태 기억




    ~MainWindow();

private slots:
    void click_calendar(const QDate &date); //서현추가


private:
    int user_id;
    int trainer_id;
    Ui::MainWindow *ui;
    QList<std_user_info> todos;      //db에서 값 받아오는거 저장하는거
    QList<QString> routine_user;    //운동루틴 적은거 저장하는 리스트

    // QList<Userbtn*> lst_wiget_namebtns;     //****************************
    QMap<QString, int>trainerMap;
    QList<QString> lst_trainer_name;         //트레이너 이름 담는 리스트
    QList<int> lst_trainer_id;               //트레이너 아이디 담는 리스트

    QList<std_user_info> lst_user_info;           //회원 정보 구조체담는 리스트
    QList<QPushButton*> lst_username_btns;              //회원이름 출력하는 버튼 담는 리스트
    QList<db_pt_data> pt_list;          //pt일정 담는 리스트 -진영
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");


    void pt_click_num(const QDate &selectedDate);
};
#endif // MAINWINDOW_H
