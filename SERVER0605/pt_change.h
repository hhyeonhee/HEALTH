#ifndef PT_CHANGE_H
#define PT_CHANGE_H

#include <QWidget>
#include <QSqlDatabase> // 추가: DB 관련 헤더 필요
#include <QSqlQuery>    // 추가: DB 관련 헤더 필요
#include <QSqlError>    // 추가: DB 관련 헤더 필요
#include <QDebug>       // 추가: 디버그 출력용
#include <QDateTime>
#include <QDate>

namespace Ui {
class pt_change;
}

class pt_change : public QWidget
{
    Q_OBJECT

public:
    explicit pt_change(QWidget *parent = nullptr, QString pt_option = "", int id = -1);


    void read_db_pt();          //db에서 수업 정보 가져오기   진영
    ~pt_change();

    // ✨ schedule_id 값을 외부에서 가져갈 수 있도록 getter 함수 추가! ✨
    int getScheduleId() const { return schedule_id; } // const를 붙이면 이 함수는 멤버 변수 값을 바꾸지 않음을 보장해!

signals:
    void finished(pt_change* widget);

private slots:
    void on_yes_clicked();
    void on_no_clicked();

private:
    Ui::pt_change *ui;
    int schedule_id; // 얘는 private으로 그대로 둬!

    void update_status_and_close(int new_status);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    int num = 0;

};
#endif // PT_CHANGE_H
