#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QTcpSocket(this))
{
    ui->setupUi(this);

    // 페이지 바뀔 때 버튼 표시 여부 설정
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        qDebug() << "페이지 인덱스:" << index;
        ui->Btn_go_main->setVisible(index != 0 && index != 1);

    });

    // 👉 초기 인덱스 고정
    ui->stackedWidget->setCurrentIndex(0);

    // 👉 현재 인덱스 기준으로 버튼 표시 여부 설정
    ui->Btn_go_main->setVisible(ui->stackedWidget->currentIndex() != 0 && ui->stackedWidget->currentIndex() != 1);


    // pt확인시
    this->db_open();


    user_info u;

    // 처음엔 비활성화
    ui->pushButton_send->setEnabled(false);

    ui->calendarWidget_2->setStyleSheet(R"(QCalendarWidget QToolButton
        {
            qproperty-icon: none;
            width: 0px;
            height: 0px;
            margin: 0px;
            padding: 0px;
        }

    )");

    connect(ui->calendarWidget_2, &QCalendarWidget::clicked, this, &MainWindow::pt_click_num);
    connect(ui->pushButton_send, &QPushButton::clicked, this, &MainWindow::pt_send);

    // 채팅관련 커넥트문
    // 메세지입력전에 입력버튼 안눌림
    ui->Btn_send_msg->setEnabled(false);
    // 버튼 → 메시지 보내기
    connect(ui->Btn_send_msg, &QPushButton::clicked,this, &MainWindow::onPushButtonClicked);

    // 페이지 넘기는 커넥트
    connect(ui->Btn_go_main,&QPushButton::clicked,this,&MainWindow::go_to_main);
    connect(ui->Btn_go_chat,&QPushButton::clicked,this,&MainWindow::go_to_chat);
    connect(ui->Btn_go_check,&QPushButton::clicked,this,&MainWindow::go_to_check);
    connect(ui->Btn_go_PT,&QPushButton::clicked,this,&MainWindow::go_to_pt);
    connect(ui->Btn_go_info,&QPushButton::clicked,this,&MainWindow::go_to_info);
    connect(ui->Btn_logout,&QPushButton::clicked,this,&MainWindow::go_to_logout);
    // connect(ui->Btn_login,&QPushButton::clicked,this,&MainWindow::go_to_login);
    // USER_EXERCISE 테이블 관련 커넥트
    connect(ui->Btn_login, &QPushButton::clicked, this, &MainWindow::user_login);
    connect(ui->calendarWidget,&QCalendarWidget::clicked,this,&MainWindow::show_my_exercise);
    connect(ui->calendarWidget_2, &QCalendarWidget::selectionChanged, this, [=]() {
        QDate selected = ui->calendarWidget_2->selectedDate();
        update_possible_day(selected);
    });

    // 2) 이번 주 월요일과 다음 주 일요일 계산
    QDate startOfWeek, endOfNextWeek;
    calcThisAndNextWeek(startOfWeek, endOfNextWeek);

    // 3) 달력에서 선택 가능한 최소/최대 날짜를 설정 (이 범위 밖은 disabled)
    ui->calendarWidget_2->setMinimumDate(startOfWeek);
    ui->calendarWidget_2->setMaximumDate(endOfNextWeek);

    // 4) 배경색을 칠할 QTextCharFormat 준비
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(QBrush(QColor(200, 230, 255)));
    // (원하는 색상으로 바꿔도 됩니다. 예: 연한 하늘색)

    // 5) 이번 주 월요일부터 다음 주 일요일까지 하루씩 순회하며 format 적용
    for (QDate d = startOfWeek; d <= endOfNextWeek; d = d.addDays(1)) {
        ui->calendarWidget_2->setDateTextFormat(d, highlightFormat);
    }

    // 6) 초기 선택 날짜를 오늘(또는 startOfWeek)로 설정
    ui->calendarWidget_2->setSelectedDate(QDate::currentDate());
    currentDate = ui->calendarWidget_2->selectedDate();

    connect(ui->radioButton_1, &QPushButton::clicked,this, &MainWindow::change_pt_daytime);
    connect(ui->radioButton_2, &QPushButton::clicked,this, &MainWindow::change_pt_daytime);
    connect(ui->radioButton_4, &QPushButton::clicked,this, &MainWindow::change_pt_daytime);
    connect(ui->radioButton_5, &QPushButton::clicked,this, &MainWindow::change_pt_daytime);
    connect(ui->radioButton_6, &QPushButton::clicked,this, &MainWindow::change_pt_daytime);



}

void calcThisAndNextWeek(QDate &outStartOfWeek, QDate &outEndOfNextWeek) {
    QDate today = QDate::currentDate();
    outStartOfWeek = today;  // 오늘부터
    outEndOfNextWeek = today.addDays(13);  // 2주 뒤까지 (오늘 포함 14일)
}


void MainWindow::show_main_btn(){
        ui->Btn_go_main->show();
}

void MainWindow::any_btn_clicked()
{
    ui->pushButton_send->setEnabled(true);  // 요청버튼 활성화

}

void MainWindow::db_open(){

    if (QSqlDatabase::contains("main_connection")) {
        QSqlDatabase::database("main_connection").isOpen();
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("10.10.20.123");
    db.setDatabaseName("HEALTH");
    db.setUserName("pipipi");
    db.setPassword("1234");
    db.setPort(3306);

    if (!db.open()) {
        qDebug() << "❌ DB 연결 실패:" << db.lastError().text();
        // return false;
    }
    qDebug() << "DB 연결 성공!";
}

void MainWindow::user_login(){

    input_id = ui->id_input->text();
    input_password = ui->password_input->text();


    // db_open();
    QSqlQuery query(QSqlDatabase::database("QMYSQL"));
    query.prepare("SELECT U_ID, U_NAME, U_BIRTH, U_TRAINER FROM USER_INFO WHERE U_NAME = :NAME"); // :블라블라 -> 파라미터는 무엇이냐 sql에 값을 직접 넣지 않고 나중에 안전하게 바인딩
    // 파라미터를 쓰는 이유는? 사용자가 드랍테이블 하면 우째요. 내부적으로 Qt가 escape 처리 및 보안 적용 해준데요.
    // U_NAME에서 사용자가 입력한 값을 가진 행만 가져와욤
    query.bindValue(":NAME", input_id); //입력 받은 값을 U_NAME에 집어 넣어

    if (!query.exec()) {
        QMessageBox::warning(this, "쿼리 실패", query.lastError().text());
        return;
    }

    if (query.next()) { //입력한 아이디의 줄만 읽어오기 때문에 아이디가 없으면 false를 반환하기 때문에 if문에 들어가지 않습니다.!!!!
        QString U_BIRTH_input = query.value("U_BIRTH").toString(); //

        if (input_password == U_BIRTH_input) {
            QMessageBox::information(this, "로그인 성공", "환영합니다!");
            u.U_TRAINER = query.value("U_TRAINER").toString();
            u.input_id_num = query.value("U_ID").toString();
            qDebug() << u.input_id_num << "요것은 로그인 한 회원 번호입니다/";
            ui->stackedWidget->setCurrentIndex(1); // 로그인 성공 하면 메인화면으로 휘리릭
            send_click_time();

        } else {
            QMessageBox::warning(this, "로그인 실패", "비밀번호가 일치하지 않습니다.");
            qDebug() << U_BIRTH_input;
        }
    } else {
        QMessageBox::warning(this, "로그인 실패", "존재하지 않는 ID입니다.");
    }
}

void MainWindow::user_info1(){

    QSqlQuery query(QSqlDatabase::database("QMYSQL"));

    query.prepare("SELECT U_ID, U_PHONENUM, U_TRAINER, U_PT_CNT FROM USER_INFO WHERE U_NAME = :input_id");
    // query.prepare("SELECT NAME FROM USER_INFO WHERE ID = :input_id");
    query.bindValue(":input_id", input_id);

    if (query.exec()) { // 문자열로 전달된 SQL을 즉시 실행, 반환값은 BOOL값!
        while (query.next()) { // 다음 값이 있으면 TRUE, 없으면 FALSE

            u.U_PHONENUM = query.value("U_PHONENUM").toString(); //구조체에 담아요.스트링
            u.U_TRAINER = query.value("U_TRAINER").toString();
            u.U_PT_CNT = query.value("U_PT_CNT").toString();
            qDebug() << "애옹1" ; //제대로 전달 됐나요? - 디버깅
            qDebug() << u.U_TRAINER;
            ui->U_NAME->setText("이름: " + input_id);
            ui->U_BIRTH->setText("생년월일: " + input_password);
            ui->U_PHONENUM->setText("전화번호: " + u.U_PHONENUM);
            ui->U_PT_CNT->setText("남은PT횟수: " + u.U_PT_CNT);
        }
    }
    // 유저 정보에서 뽑아온 트레이너넘버로 트레이너이름 불러오기
    query.prepare("SELECT NAME FROM TRAINER_INFO WHERE ID = :t_name");
    query.bindValue(":t_name", u.U_TRAINER);

    if (query.exec()) {
        while (query.next()) {
            u.U_TRAINER_NAME = query.value("NAME").toString();

            ui->U_TRAINER->setText("담당트레이너: " + u.U_TRAINER_NAME);
        }
    }
    else {
        qDebug() << "쿼리 실패애옹?:" << query.lastError().text();
    }
}

void MainWindow::user_info2(){

    QLabel* qlabels[10];
    qlabels[0] = ui->label_1;
    qlabels[1] = ui->label_2;
    qlabels[2] = ui->label_3;
    qlabels[3] = ui->label_4;
    qlabels[4] = ui->label_5;
    qlabels[5] = ui->label_6;
    qlabels[6] = ui->label_7;
    qlabels[7] = ui->label_9;
    qlabels[8] = ui->label_10;
    qlabels[9] = ui->label_11;
    // qlabels[10] = ui->label_11;

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[show_my_exercise] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        qDebug() << "[show_my_exercise] DB가 닫혀 있습니다. 재연결 시도...";
        if (!db.open()) {
            qDebug() << "[show_my_exercise] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }
    QSqlQuery query(QSqlDatabase::database("QMYSQL"));

    //"SELECT U_ID, U_PHONENUM, U_TRAINER, U_PT_CNT FROM USER_INFO WHERE U_NAME = :input_id"
    query.prepare("SELECT PT_TIME, PT_DAY, STATUS FROM PT_SCHEDULE WHERE USER_ID = :input_id_num");
    query.bindValue(":input_id_num", u.input_id_num);
    qDebug() << u.input_id_num;

    if (query.exec()) {
        int index = 0;
        while (query.next() && index < 10) {


            int status_num =  query.value("STATUS").toInt();
            QString pt_time = query.value("PT_TIME").toString();
            QString pt_day = query.value("PT_DAY").toDateTime().toString("yyyy-MM-dd");


            QString text = "일자 : " + pt_day + " | 시간 : " + pt_time + "시  -  " + "PT " + statusToString(status_num);
            qlabels[index]->setText(text);
            index++;
        }

    }
    else {
        qDebug() << "쿼리 실패애옹?:" << query.lastError().text();
    }

}

// 회원 예약창 함수
void MainWindow::show_my_exercise(const QDate &selectedDate)
{
    // ─────────────────────────────────────────────────────────────────────────
    // (1) 지금 클릭된 날짜를 멤버 변수에 저장
    currentDate = selectedDate;
    // ─────────────────────────────────────────────────────────────────────────

    // ─────────────────────────────────────────────────────────────────────────
    // (2) 우선 화면의 모든 체크박스를 초기화(빈 텍스트 + 언체크)
    for (int i = 1; i <= 6; ++i) {
        QRadioButton *cb = this->findChild<QRadioButton*>(QString("checkBox_exe%1").arg(i));
        if (cb) {
            cb->setText("");
            cb->setChecked(false);
        }
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─────────────────────────────────────────────────────────────────────────
    // (3) DB 커넥션 확인 및 재연결
    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[show_my_exercise] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        qDebug() << "[show_my_exercise] DB가 닫혀 있습니다. 재연결 시도...";
        if (!db.open()) {
            qDebug() << "[show_my_exercise] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }
    // ─────────────────────────────────────────────────────────────────────────

    // ─────────────────────────────────────────────────────────────────────────
    // (4) 클릭된 날짜를 "yyyy-MM-dd" 문자열로 변환
    QString dateString = selectedDate.toString("yyyy-MM-dd");
    qDebug() << "[show_my_exercise] 선택된 날짜 =" << dateString;
    // ─────────────────────────────────────────────────────────────────────────

    // ─────────────────────────────────────────────────────────────────────────
    // (5) SELECT 쿼리: U_ID=1(테스트용) & DATE(EXERCISE_DATE)=dateString
    QString sql = QString(
                      "SELECT EXERCISE1, EXERCISE2, EXERCISE3, EXERCISE4, EXERCISE5, EXERCISE6, "
                      "       EXERCISE1_CHECK, EXERCISE2_CHECK, EXERCISE3_CHECK, EXERCISE4_CHECK, EXERCISE5_CHECK, EXERCISE6_CHECK "
                      "FROM USER_EXERCISE "
                      "WHERE U_ID = %1 AND DATE(EXERCISE_DATE) = '%2' "
                      "LIMIT 1"
                      ).arg(u.input_id_num).arg(dateString);

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qDebug() << "[show_my_exercise] 쿼리 실행 실패:" << query.lastError().text();
        return;
    }
    qDebug() << "[show_my_exercise] 쿼리 실행 성공 (U_ID=1, 날짜=" << dateString << ")";
    // ─────────────────────────────────────────────────────────────────────────

    // ─────────────────────────────────────────────────────────────────────────
    // (6) 결과가 있으면, 화면의 체크박스를 DB 값으로 덮어쓰기
    if (query.next()) {
        // 컬럼 인덱스: 0~5 → EXERCISE1~6, 6~11 → EXERCISE1_CHECK~6_CHECK
        QString e1 = query.value(0).toString();
        QString e2 = query.value(1).toString();
        QString e3 = query.value(2).toString();
        QString e4 = query.value(3).toString();
        QString e5 = query.value(4).toString();
        QString e6 = query.value(5).toString();

        bool c1 = query.value(6).toInt() == 1;
        bool c2 = query.value(7).toInt() == 1;
        bool c3 = query.value(8).toInt() == 1;
        bool c4 = query.value(9).toInt() == 1;
        bool c5 = query.value(10).toInt() == 1;
        bool c6 = query.value(11).toInt() == 1;

        ui->checkBox_exe1->setText(e1);
        ui->checkBox_exe1->setChecked(c1);

        ui->checkBox_exe2->setText(e2);
        ui->checkBox_exe2->setChecked(c2);

        ui->checkBox_exe3->setText(e3);
        ui->checkBox_exe3->setChecked(c3);

        ui->checkBox_exe4->setText(e4);
        ui->checkBox_exe4->setChecked(c4);

        ui->checkBox_exe5->setText(e5);
        ui->checkBox_exe5->setChecked(c5);

        ui->checkBox_exe6->setText(e6);
        ui->checkBox_exe6->setChecked(c6);

        qDebug() << "[show_my_exercise] UI에 데이터 반영 완료";
    }
    // (없으면, 이미 초기화되어 있으므로 그대로 빈 상태)
}

// 현재 dateString, U_ID 조합으로 레코드 존재 여부 확인
bool recordExists(int uId, const QString &dateString, QSqlDatabase &db) {
    // DATE(EXERCISE_DATE) = 'YYYY-MM-DD' 로 비교
    QSqlQuery q(db);
    QString sql = QString(
                      "SELECT COUNT(*) FROM USER_EXERCISE "
                      "WHERE U_ID = %1 AND DATE(EXERCISE_DATE) = '%2'"
                      ).arg(uId).arg(dateString);

    if (!q.exec(sql)) {
        qDebug() << "[recordExists] 쿼리 실패:" << q.lastError().text();
        return false;
    }
    if (q.next()) {
        return (q.value(0).toInt() > 0);
    }
    return false;
}
void MainWindow::on_checkBox_exe1_toggled(bool checked)
{
    // 1) U_ID, 현재 날짜 문자열 가져오기
    int input_id_num_num = u.input_id_num.toInt();

    QString dateString = currentDate.toString("yyyy-MM-dd");
    qDebug() << "[on_checkBox_exe1_toggled] 현재 날짜 =" << dateString << ", 체크 상태 =" << checked;

    // 2) 커넥션 확인
    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[on_checkBox_exe1_toggled] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "[on_checkBox_exe1_toggled] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    // 3) 해당 날짜 레코드 존재 여부 확인
    bool exists = recordExists(input_id_num_num, dateString, db);

    QSqlQuery query(db);
    int newValue = checked ? 1 : 0;

    if (exists) {
        // 4-1) 레코드가 이미 있으면 UPDATE
        QString sql = QString(
                          "UPDATE USER_EXERCISE "
                          "SET EXERCISE1_CHECK = %1 "
                          "WHERE U_ID = %2 "
                          "  AND DATE(EXERCISE_DATE) = '%3'"
                          ).arg(newValue).arg(u.input_id_num).arg(dateString);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe1_toggled] UPDATE 실패:" << query.lastError().text();
        } else {
            qDebug() << "[on_checkBox_exe1_toggled] EXERCISE1_CHECK 업데이트 성공 ->" << newValue;
        }
    } else {
        // 4-2) 레코드가 없으면 INSERT (텍스트 컬럼은 빈 문자열로 넣거나 NULL로 남김)
        //     EXERCISE1만 예외적으로 채우고, 나머지 컬럼은 기본값(NULL)으로 삽입
        QString sql = QString(
                          "INSERT INTO USER_EXERCISE "
                          "(U_ID, EXERCISE_DATE, EXERCISE1, EXERCISE1_CHECK) "
                          "VALUES (%1, '%2', '%3', %4)"
                          ).arg(u.input_id_num)
                          .arg(dateString + " 00:00:00")   // 새 레코드는 자정 시각으로 EXERCISE_DATE 설정
                          .arg(ui->checkBox_exe1->text())  // 텍스트가 이미 화면에 있다면 그대로 넣거나, 빈 문자열
                          .arg(newValue);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe1_toggled] INSERT 실패:" << query.lastError().text();
        } else {
            qDebug() << "[on_checkBox_exe1_toggled] 레코드 추가 & EXERCISE1_CHECK 설정 ->" << newValue;
        }
    }
}
// 나머지 체크박스도 같은 로직으로 구현
void MainWindow::on_checkBox_exe2_toggled(bool checked)
{
    int input_id_num_num = u.input_id_num.toInt();
    QString dateString = currentDate.toString("yyyy-MM-dd");

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[on_checkBox_exe2_toggled] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "[on_checkBox_exe2_toggled] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    bool exists = recordExists(input_id_num_num, dateString, db);
    QSqlQuery query(db);
    int newValue = checked ? 1 : 0;

    if (exists) {
        QString sql = QString(
                          "UPDATE USER_EXERCISE "
                          "SET EXERCISE2_CHECK = %1 "
                          "WHERE U_ID = %2 AND DATE(EXERCISE_DATE) = '%3'"
                          ).arg(newValue).arg(u.input_id_num).arg(dateString);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe2_toggled] UPDATE 실패:" << query.lastError().text();
        } else {
            qDebug() << "[on_checkBox_exe2_toggled] EXERCISE2_CHECK 업데이트 성공 ->" << newValue;
        }
    } else {
        QString sql = QString(
                          "INSERT INTO USER_EXERCISE "
                          "(U_ID, EXERCISE_DATE, EXERCISE2, EXERCISE2_CHECK) "
                          "VALUES (%1, '%2', '%3', %4)"
                          ).arg(u.input_id_num)
                          .arg(dateString + " 00:00:00")
                          .arg(ui->checkBox_exe2->text())
                          .arg(newValue);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe2_toggled] INSERT 실패:" << query.lastError().text();
        } else {
            qDebug() << "[on_checkBox_exe2_toggled] 레코드 추가 & EXERCISE2_CHECK 설정 ->" << newValue;
        }
    }
}
void MainWindow::on_checkBox_exe3_toggled(bool checked)
{
    int input_id_num_num = u.input_id_num.toInt();
    QString dateString = currentDate.toString("yyyy-MM-dd");
    qDebug() << "[on_checkBox_exe3_toggled] 날짜 =" << dateString << ", 체크 =" << checked;

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[on_checkBox_exe3_toggled] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "[on_checkBox_exe3_toggled] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    bool exists = recordExists(input_id_num_num, dateString, db);
    QSqlQuery query(db);
    int newValue = checked ? 1 : 0;

    if (exists) {
        QString sql = QString(
                          "UPDATE USER_EXERCISE "
                          "SET EXERCISE3_CHECK = %1 "
                          "WHERE U_ID = %2 AND DATE(EXERCISE_DATE) = '%3'"
                          ).arg(newValue).arg(u.input_id_num).arg(dateString);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe3_toggled] UPDATE 실패:" << query.lastError().text();
        }
    } else {
        QString sql = QString(
                          "INSERT INTO USER_EXERCISE "
                          "(U_ID, EXERCISE_DATE, EXERCISE3, EXERCISE3_CHECK) "
                          "VALUES (%1, '%2', '%3', %4)"
                          ).arg(u.input_id_num)
                          .arg(dateString + " 00:00:00")
                          .arg(ui->checkBox_exe3->text())
                          .arg(newValue);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe3_toggled] INSERT 실패:" << query.lastError().text();
        }
    }
}
void MainWindow::on_checkBox_exe4_toggled(bool checked)
{
    int input_id_num_num = u.input_id_num.toInt();
    QString dateString = currentDate.toString("yyyy-MM-dd");
    qDebug() << "[on_checkBox_exe4_toggled] 날짜 =" << dateString << ", 체크 =" << checked;

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[on_checkBox_exe4_toggled] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "[on_checkBox_exe4_toggled] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    bool exists = recordExists(input_id_num_num, dateString, db);
    QSqlQuery query(db);
    int newValue = checked ? 1 : 0;

    if (exists) {
        QString sql = QString(
                          "UPDATE USER_EXERCISE "
                          "SET EXERCISE4_CHECK = %1 "
                          "WHERE U_ID = %2 AND DATE(EXERCISE_DATE) = '%3'"
                          ).arg(newValue).arg(u.input_id_num).arg(dateString);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe4_toggled] UPDATE 실패:" << query.lastError().text();
        }
    } else {
        QString sql = QString(
                          "INSERT INTO USER_EXERCISE "
                          "(U_ID, EXERCISE_DATE, EXERCISE4, EXERCISE4_CHECK) "
                          "VALUES (%1, '%2', '%3', %4)"
                          ).arg(u.input_id_num)
                          .arg(dateString + " 00:00:00")
                          .arg(ui->checkBox_exe4->text())
                          .arg(newValue);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe4_toggled] INSERT 실패:" << query.lastError().text();
        }
    }
}
void MainWindow::on_checkBox_exe5_toggled(bool checked)
{
    int input_id_num_num = u.input_id_num.toInt();
    QString dateString = currentDate.toString("yyyy-MM-dd");
    qDebug() << "[on_checkBox_exe5_toggled] 날짜 =" << dateString << ", 체크 =" << checked;

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[on_checkBox_exe5_toggled] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "[on_checkBox_exe5_toggled] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    bool exists = recordExists(input_id_num_num, dateString, db);
    QSqlQuery query(db);
    int newValue = checked ? 1 : 0;

    if (exists) {
        QString sql = QString(
                          "UPDATE USER_EXERCISE "
                          "SET EXERCISE5_CHECK = %1 "
                          "WHERE U_ID = %2 AND DATE(EXERCISE_DATE) = '%3'"
                          ).arg(newValue).arg(u.input_id_num).arg(dateString);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe5_toggled] UPDATE 실패:" << query.lastError().text();
        }
    } else {
        QString sql = QString(
                          "INSERT INTO USER_EXERCISE "
                          "(U_ID, EXERCISE_DATE, EXERCISE5, EXERCISE5_CHECK) "
                          "VALUES (%1, '%2', '%3', %4)"
                          ).arg(u.input_id_num)
                          .arg(dateString + " 00:00:00")
                          .arg(ui->checkBox_exe5->text())
                          .arg(newValue);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe5_toggled] INSERT 실패:" << query.lastError().text();
        }
    }
}
void MainWindow::on_checkBox_exe6_toggled(bool checked)
{
    int input_id_num_num = u.input_id_num.toInt();
    QString dateString = currentDate.toString("yyyy-MM-dd");
    qDebug() << "[on_checkBox_exe6_toggled] 날짜 =" << dateString << ", 체크 =" << checked;

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[on_checkBox_exe6_toggled] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        if (!db.open()) {
            qDebug() << "[on_checkBox_exe6_toggled] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    bool exists = recordExists(input_id_num_num, dateString, db);
    QSqlQuery query(db);
    int newValue = checked ? 1 : 0;

    if (exists) {
        QString sql = QString(
                          "UPDATE USER_EXERCISE "
                          "SET EXERCISE6_CHECK = %1 "
                          "WHERE U_ID = %2 AND DATE(EXERCISE_DATE) = '%3'"
                          ).arg(newValue).arg(u.input_id_num).arg(dateString);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe6_toggled] UPDATE 실패:" << query.lastError().text();
        }
    } else {
        QString sql = QString(
                          "INSERT INTO USER_EXERCISE "
                          "(U_ID, EXERCISE_DATE, EXERCISE6, EXERCISE6_CHECK) "
                          "VALUES (%1, '%2', '%3', %4)"
                          ).arg(u.input_id_num)
                          .arg(dateString + " 00:00:00")
                          .arg(ui->checkBox_exe6->text())
                          .arg(newValue);

        if (!query.exec(sql)) {
            qDebug() << "[on_checkBox_exe6_toggled] INSERT 실패:" << query.lastError().text();
        }
    }
}
//페이지 넘기는 함수들
void MainWindow::go_to_main(){
    ui->stackedWidget->setCurrentIndex(1);
    // ui->Btn_go_main->setVisible(false);

}
void MainWindow::go_to_chat(){
    ui->stackedWidget->setCurrentIndex(2);
    // 소켓 시그널
    connect(socket, &QTcpSocket::connected,this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::readyRead,this, &MainWindow::onReadyRead);
    connect(socket,qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred),this, &MainWindow::onErrorOccurred);
    // 서버에 접속
    socket->connectToHost("10.10.20.122", 12345);
};
void MainWindow::go_to_check(){
    ui->stackedWidget->setCurrentIndex(3);
};
void MainWindow::go_to_pt(){


    ui->stackedWidget->setCurrentIndex(4);
    pt_schedule();
    ui->radioButton_1->setEnabled(true);
    ui->radioButton_2->setEnabled(true);
    ui->radioButton_3->setEnabled(true);
    ui->radioButton_4->setEnabled(true);
    ui->radioButton_5->setEnabled(true);
    ui->radioButton_6->setEnabled(true);

    if(ui->radioButton_1->text().isEmpty()){
        ui->radioButton_1->setEnabled(false);
    }
    if(ui->radioButton_2->text().isEmpty()){
        ui->radioButton_2->setEnabled(false);
    }
    if(ui->radioButton_3->text().isEmpty()){
        ui->radioButton_3->setEnabled(false);
    }
    if(ui->radioButton_4->text().isEmpty()){
        ui->radioButton_4->setEnabled(false);
    }
    if(ui->radioButton_5->text().isEmpty()){
        ui->radioButton_5->setEnabled(false);
    }
    if(ui->radioButton_6->text().isEmpty()){
        ui->radioButton_6->setEnabled(false);
    }

    ui->pushButton_send->setEnabled(false);
    if (u.selected_Button_Index >= 10 && u.selected_Button_Index <= 21) { //버튼 눌렀을 때
        ui->pushButton_send->setEnabled(true);  // 버튼 활성화
    } else {
        ui->pushButton_send->setEnabled(false); // 버튼 비활성화
    }
    ui->pushButton_send->setEnabled(false); // 예약 완료 후 다시 비활성화

    QDate selected = ui->calendarWidget_2->selectedDate();
    update_possible_day(selected);
};
void MainWindow::go_to_info(){
    ui->stackedWidget->setCurrentIndex(5);
    user_info1();
    user_info2();
};
void MainWindow::go_to_logout(){
    ui->stackedWidget->setCurrentIndex(0);
    ui->id_input->clear();
    ui->password_input->clear();
    ui->Btn_go_main->hide();
}
void MainWindow::go_to_login(){
    ui->stackedWidget->setCurrentIndex(1);
    ui->Btn_go_main->setVisible(false);  // index 1에서는 무조건 숨김
}
//

// 채팅관련함수
void MainWindow::onErrorOccurred(QAbstractSocket::SocketError) {
    appendMessage(ui->verticalLayout_9,
                  "[오류] " + socket->errorString());
}
void MainWindow::onConnected() {
    socket->write(input_id.toUtf8() + '\n');  // 회원 ID 전송
    ui->Btn_send_msg->setEnabled(true);
    appendMessage(ui->verticalLayout_9, "[시스템] 서버 연결됨");
}

void MainWindow::onReadyRead() {
    while (socket->canReadLine()) {
        QString msg = QString::fromUtf8(socket->readLine()).trimmed();
        appendMessage(ui->verticalLayout_9, msg);
    }
}

void MainWindow::onPushButtonClicked() {
    QString msg = ui->lineEdit->text().trimmed();
    if (msg.isEmpty()) return;

    QString fullMsg = msg;
    QByteArray data = fullMsg.toUtf8() + '\n';

    appendMessage(ui->verticalLayout_9,"[🙋나] " + msg);
    socket->write(data);
    ui->lineEdit->clear();
}
// 레이아웃찾아가는 함수인데 좀더 공부해야함
void MainWindow::appendMessage(QVBoxLayout *layout, const QString &text) {
    QString timeStr = QDateTime::currentDateTime().toString("MM-dd HH:mm");
    QLabel *lbl = new QLabel("[" + timeStr + "] " + text, this);

    QScrollBar *vbar = ui->user_chat_scroll->verticalScrollBar();
    connect(vbar, &QScrollBar::rangeChanged, this, [vbar](int /*min*/, int max){
        vbar->setValue(max);
    });

    lbl->setWordWrap(true);
    layout->addWidget(lbl);

    QWidget *content = layout->parentWidget();
    if (content) {
        QScrollArea *area = qobject_cast<QScrollArea*>(content->parentWidget());
        if (area) {
            area->verticalScrollBar()->setValue(area->verticalScrollBar()->maximum());
        }
    }
}

// 피티 날짜 시간 선택, 예약된 라디오 버튼 선택 함수
void MainWindow::pt_schedule(){

    QRadioButton* radioButtones[6];
    radioButtones[0] = ui->radioButton_1;
    radioButtones[1] = ui->radioButton_2;
    radioButtones[2] = ui->radioButton_3;
    radioButtones[3] = ui->radioButton_4;
    radioButtones[4] = ui->radioButton_5;
    radioButtones[5] = ui->radioButton_6;


    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[show_my_exercise] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        qDebug() << "[show_my_exercise] DB가 닫혀 있습니다. 재연결 시도...";
        if (!db.open()) {
            qDebug() << "[show_my_exercise] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }
    QSqlQuery query(QSqlDatabase::database("QMYSQL"));

    query.prepare("SELECT ID, PT_TIME, PT_DAY FROM PT_SCHEDULE WHERE USER_ID = :input_id_num AND STATUS = 3");
    query.bindValue(":input_id_num", u.input_id_num);
    qDebug() << u.input_id_num;


    if (query.exec()) {
        int index = 0;
        while (query.next() && index < 6) {
            pt_id.push_back(query.value("ID").toInt());
            QString pt_time = query.value("PT_TIME").toString();
            QString pt_day = query.value("PT_DAY").toDateTime().toString("yyyy-MM-dd");
            QString text = "일자: " + pt_day + " | 시간: " + pt_time + "시";
            radioButtones[index]->setText(text);
            index++;
        }
    }
    else {
        qDebug() << "쿼리 실패애옹?:" << query.lastError().text();
    }

}
// 예약 완료 된 피티 사용자 고유 아이디 저장
void MainWindow::change_pt_daytime(){
    // int send_pt_id = 0;
    if (ui->radioButton_1->isChecked()){
        send_pt_id = pt_id[0];
    }
    if (ui->radioButton_2->isChecked()){
        send_pt_id = pt_id[1];
    }
    if (ui->radioButton_4->isChecked()){
        send_pt_id = pt_id[2];
    }
    if (ui->radioButton_5->isChecked()){
        send_pt_id = pt_id[3];
    }
    if (ui->radioButton_6->isChecked()){
        send_pt_id = pt_id[4];
    }
    signalReceived = true;
}
// 선택한 날짜 저장 함수
void MainWindow::pt_click_num(const QDate &selectedDate){

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[show_my_exercise] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        qDebug() << "[show_my_exercise] DB가 닫혀 있습니다. 재연결 시도...";
        if (!db.open()) {
            qDebug() << "[show_my_exercise] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    u.dateString = selectedDate.toString("yyyy-MM-dd");
    qDebug() << "[show_my_exercise] 선택된 날짜 =" << u.dateString;

}
// 예약 완료 버튼 누르면 status 값 변경 쿼리 함수
void MainWindow::pt_send() {
    QList<int> lst_status;

    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[pt_send] 기본 커넥션이 없습니다.";
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");

    if (!db.isOpen()) {
        qDebug() << "[pt_send] DB가 닫혀 있습니다. 재연결 시도...";
        if (!db.open()) {
            qDebug() << "[pt_send] DB 재연결 실패:" << db.lastError().text();
            return;
        }
    }

    // 위에서 DB 연결은 보장됨
    QSqlQuery query2(QSqlDatabase::database("QMYSQL"));
    QString sql = QString("SELECT STATUS FROM PT_SCHEDULE WHERE USER_ID = \"%1\" AND STATUS IN (1, 3)").arg(u.input_id_num);

    if (!query2.exec(sql)) {
        qDebug() << "[pt_send] 상태 조회 쿼리 실패:" << query2.lastError().text();
    } else {
        while (query2.next()) {
            lst_status.append(query2.value(0).toInt());
            qDebug() << "상태 리스트:" << lst_status;
        }
    }

    if (lst_status.size() == 6) {
        QMessageBox::information(this, "알림", "단위기간 내의 예약이 6개를 초과했어요!");
    } else {
        QMessageBox::information(this, "알림", "예약 요청을 보냈습니다!");

        if (u.dateString.isNull()) {
            QDate today = QDate::currentDate();
            u.dateString = today.toString("yyyy-MM-dd");
            qDebug() << "[pt_send] 기본 날짜 설정:" << u.dateString;
        }

        ui->stackedWidget->setCurrentIndex(1);

        QSqlQuery query(QSqlDatabase::database("QMYSQL"));
        query.prepare("INSERT INTO PT_SCHEDULE (USER_ID, TRAINER_ID, PT_DAY, PT_TIME, STATUS) "
                      "VALUES (?, ?, ?, ?, ?)");

        query.addBindValue(u.input_id_num);
        query.addBindValue(u.U_TRAINER);
        query.addBindValue(u.dateString);
        query.addBindValue(u.selected_Button_Index);
        query.addBindValue(1);  // STATUS = 1 (요청 상태)

        if (!query.exec()) {
            qDebug() << "[pt_send] INSERT 쿼리 실패:" << query.lastError().text();
            qDebug() << "[pt_send] 날짜 확인:" << u.dateString;
            return;
        }

        if (signalReceived) {
            change_pt_daytime_real();
            signalReceived = false;
        }
    }
}

// 예약 완료 시간을 변경, status 값 변경 함수
void MainWindow::change_pt_daytime_real(){

    QSqlQuery query(QSqlDatabase::database("QMYSQL"));
    query.prepare("UPDATE PT_SCHEDULE SET STATUS = 4 WHERE ID = ?");

    query.addBindValue(send_pt_id);
    qDebug() << send_pt_id;
    qDebug() << pt_id[0];

    if (query.exec()) {
        qDebug() << "뭐 어쩌라고:";
    }
    else {
        qDebug() << "쿼리 실패다옹옹?:" << query.lastError().text();
    }

}
// 담당 트레이너의 예약 가능 시간을 버튼 활성화 함수
void MainWindow::update_possible_day(const QDate &date) {
    QString dayOfWeek = date.toString("ddd").toUpper();  // 예: "MON", "TUE"

    QMap<QString, QString> tableMap = {
        {"MON", "MON_INFO"}, {"TUE", "TUE_INFO"}, {"WED", "WED_INFO"},
        {"THU", "THU_INFO"}, {"FRI", "FRI_INFO"}, {"SAT", "SAT_INFO"}, {"SUN", "SUN_INFO"}
    };

    QString tableName = tableMap.value(dayOfWeek);
    if (tableName.isEmpty()) return;

    int trainerId = u.U_TRAINER.toInt();
    qDebug() << "타입 확인:" << u.U_TRAINER << "=>" << u.U_TRAINER.toInt();

    QSqlQuery query(QSqlDatabase::database("pt_connection"));
    QString sql = QString("SELECT * FROM %1 WHERE ID = '%2'")
                      .arg(tableName)
                      .arg(trainerId);
    if (!query.exec(sql)) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return;
    }

    if (query.next()) {
        for (int hour = 10; hour <= 21; ++hour) {
            QString column = QString("POSSIBLE_%1").arg(hour);
            bool available = query.value(column).toInt() == 1;

            QString btnName = QString("pushButton_%1").arg(hour);
            QPushButton *btn = findChild<QPushButton*>(btnName);
            if (btn) btn->setEnabled(available);
        }
    }
}
// 예약 시간 클릭 저장 함수
void MainWindow::send_click_time(){

    for (int i = 10; i <= 21; ++i) {
        QString btnName = QString("pushButton_%1").arg(i);
        QPushButton *btn = findChild<QPushButton *>(btnName);
        if (btn) {
            connect(btn, &QPushButton::clicked, this, [&, i]() {
                u.selected_Button_Index = i;
                qDebug() << "이것은 내가 누른 시간이도다 : " << u.selected_Button_Index ;
                ui->pushButton_send->setEnabled(true);

            });
        }
    }
}








//
MainWindow::~MainWindow()
{
    delete ui;
}
