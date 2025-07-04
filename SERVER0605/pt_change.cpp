#include "pt_change.h"
#include "ui_pt_change.h"

pt_change::pt_change(QWidget *parent, QString pt_option, int id)
    : QWidget(parent)
    , ui(new Ui::pt_change)
    , schedule_id(id) // 생성자에서 schedule_id 초기화
{
    ui->setupUi(this);
    ui->textBrowser->append(pt_option);

    // 버튼 클릭 시 슬롯 연결
    connect(ui->Btn_yes, &QPushButton::clicked, this, &pt_change::on_yes_clicked);
    connect(ui->Btn_no, &QPushButton::clicked, this, &pt_change::on_no_clicked);

    // 👍 메모리 관리: pt_change 위젯이 삭제될 때 delete ui; 호출하도록 설정
    setAttribute(Qt::WA_DeleteOnClose);
}

// 상태 업데이트 후 MainWindow에게 완료를 알리는 함수
void pt_change::update_status_and_close(int new_status) {
    QSqlDatabase db = QSqlDatabase::database("pt_connection");
    if (!db.isOpen()) {
        qDebug() << "DB가 닫혀 있음!";
        // DB 연결 실패 시에도 시그널은 보내서 위젯이 사라지도록 할 수도 있어.
        // 여기선 일단 실패 메시지만 찍고 리턴. 필요하다면 아래 emit 라인 추가 고려.
        // emit finished(this);
        return;
    }

    QList<int> idList;
    // QList<int> stList;

    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT ID, STATUS FROM PT_SCHEDULE WHERE STATUS IN (1, 4)");

    if (!selectQuery.exec()) {
        qDebug() << "쿼리 실패:" << selectQuery.lastError().text();
        return;
    }

    while (selectQuery.next()) {
        int status = selectQuery.value("STATUS").toInt();
        int schedule_id = selectQuery.value("ID").toInt();
        idList.append(schedule_id);
        // stList.append(status);

        qDebug() << "현재 상태:" << status;

        QSqlQuery updateQuery(db);
        if (status == 4) {
            if (this->num == 1) {
                updateQuery.prepare("UPDATE PT_SCHEDULE SET STATUS = 5 WHERE ID = :id");    // 수락 시 5
            } else {
                updateQuery.prepare("UPDATE PT_SCHEDULE SET STATUS = 3 WHERE ID = :id");    // 거절 시 3
            }
        } else if (status == 1) {
            updateQuery.prepare("UPDATE PT_SCHEDULE SET STATUS = :status WHERE ID = :id");
            updateQuery.bindValue(":status", new_status);  // new_status는 외부에 정의되어 있다고 가정
        }

        updateQuery.bindValue(":id", schedule_id);

        if (!updateQuery.exec()) {
            qDebug() << "업데이트 실패 (ID:" << schedule_id << "):" << updateQuery.lastError().text();
        } else {
            qDebug() << "업데이트 성공 (ID:" << schedule_id << ")";
        }
    }




    // ✨ deleteLater() 대신 finished 시그널을 내보내서 MainWindow에게 나를 지워달라고 부탁! ✨
    emit finished(this);
    // 이 위젯은 MainWindow의 슬롯에서 deleteLater()를 통해 삭제될 거야.
}
//이거야ㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑ
void pt_change::read_db_pt() {      //예약잡히면 해당하는 요일
    QSqlDatabase db = QSqlDatabase::database("pt_connection");  // 기존 연결 사용
    if (!db.isOpen()) {
        qDebug() << "DB가 닫혀 있음!";
        return;
    }

    QSqlQuery query(db);
    QString sql = "SELECT TRAINER_ID, PT_DAY, PT_TIME, STATUS FROM PT_SCHEDULE"; // DB에서 아이디, 일, 시간, 상태 가져오기
    if (query.exec(sql)) {
        while (query.next()) { // 하나하나 DB에서 가져온 값 담기
            int trainerId  = query.value("TRAINER_ID").toInt();
            QDateTime ptDay = query.value("PT_DAY").toDateTime();
            int ptTime     = query.value("PT_TIME").toInt();
            int status     = query.value("STATUS").toInt();

            // 요일 구하기
            QDate date = ptDay.date();                 // DATE타입에서 날짜 추출
            int dayOfWeek = date.dayOfWeek();          // 1 = 월 ~ 7 = 일

            QString tableName;
            switch (dayOfWeek) {
            case 1: tableName = "MON_INFO"; break;
            case 2: tableName = "TUE_INFO"; break;
            case 3: tableName = "WED_INFO"; break;
            case 4: tableName = "THU_INFO"; break;
            case 5: tableName = "FRI_INFO"; break;
            case 6: tableName = "SAT_INFO"; break;
            case 7: tableName = "SUN_INFO"; break;
            }

            // 디버그 출력
            qDebug() << "PT_SCHEDULE 날짜:" << ptDay.toString("yyyy-MM-dd")
                     << ", 요일:" << tableName
                     << ", 트레이너:" << trainerId
                     << ", 시간:" << ptTime
                     << ", 상태:" << status;

            // 요일별 테이블에 UPDATE
            QString timeColumn = QString("POSSIBLE_%1").arg(ptTime);

            QString updateSql = QString("UPDATE %1 SET %2 = 0 WHERE ID = :trainer")
                                    .arg(tableName)
                                    .arg(timeColumn);

            QSqlQuery updateQuery(db);
            updateQuery.prepare(updateSql);
            updateQuery.bindValue(":trainer", trainerId);

            qDebug() << "sql: " << updateSql;
            qDebug() << "trainerId: " << trainerId;
            if (!updateQuery.exec()) {
                qDebug() << "업데이트 실패?" << db.lastError().text();;
            } else {
                qDebug() << "업데이트 성공";
            }
        }
    } else {
        qDebug() << "[쿼리 실패]" << query.lastError().text();
    }
}

void pt_change::on_yes_clicked() {
    this->num = 1;
    update_status_and_close(3); // YES → 상태 3
    read_db_pt();       //상태 바꾸는 함수로
}

void pt_change::on_no_clicked() {
    update_status_and_close(2); // 상태를 2로 변경하고 알림
}

pt_change::~pt_change()
{
    // deleteLater()가 호출되면 Qt가 알아서 소멸자를 호출해줘.
    // ui는 new로 할당했으니 소멸자에서 delete 해주는 게 맞아!
    delete ui;
    qDebug() << "pt_change 위젯 삭제됨. ID:" << schedule_id; // 디버깅용 메시지
}
