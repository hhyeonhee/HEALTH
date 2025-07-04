#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0); //초기화면 무조건 0번

    // 페이지 바뀔 때 버튼 표시 여부 설정
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        // index 0: 메인화면일 때 버튼 숨기기
        ui->Btn_go_main->setVisible(index != 0);
    });

    // 초기 화면에서도 반영되게 설정
    ui->Btn_go_main->setVisible(ui->stackedWidget->currentIndex() != 0);

    this->setDB();
    this->trainer_info();

    connect(ui->Btn_go_main, &QPushButton::clicked, this, &MainWindow::go_to_page_main);
    connect(ui->Btn_go_clnt_info, &QPushButton::clicked, this, &MainWindow::go_to_page_clnt);
    connect(ui->Btn_go_exe_check, &QPushButton::clicked, this, &MainWindow::go_to_page_check);
    connect(ui->Btn_go_close, &QPushButton::clicked, this, &MainWindow::go_to_page_close);
    connect(ui->userin_save_Btn, &QPushButton::clicked, this, &MainWindow::set_user_info_Btn);
    connect(ui->save_Btn, &QPushButton::clicked, this, &MainWindow::routine_write_db);
    connect(ui->deadline_Btn, &QPushButton::clicked, this, &MainWindow::deadline_db);
    connect(ui->calendarWidget, &QCalendarWidget::clicked, this, &MainWindow::pt_click_num);
    connect(ui->calendarWidget, &QCalendarWidget::clicked, this, &MainWindow::click_calendar); //추~가
    connect(ui->Btn_go_userin, &QPushButton::clicked, this, &MainWindow::go_to_page_userin);    //추가 6월 3일 회원
    connect(ui->userin_save_Btn_2, &QPushButton::clicked, this, &MainWindow::new_user_insert);
    connect(ui->Btn_go_clnt_info, &QPushButton::clicked, this, &MainWindow::trainer_user);

}

//스택페이지 이동함수
void MainWindow::go_to_page_main(){
    //페이지이동
    ui->stackedWidget->setCurrentIndex(0);
    //회원가입 입력창 초기화
    ui->name_lineEdit->clear();
    ui->ph_lineEdit->clear();
    ui->bt_lineEdit->clear();
    ui->tr_combobox->setCurrentIndex(0);
    ui->pt_spinbox->setValue(0);
    clear_lineEdits();
}
void MainWindow::go_to_page_clnt(){
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::go_to_page_check(){
    ui->stackedWidget->setCurrentIndex(3);
    this->get_user_info();
    this->trBtn_handling();//******************

    QDate today = QDate::currentDate();
    dateString = today.toString("yyyy-MM-dd");
    qDebug() << "오늘 날짜임: " << dateString;
}
void MainWindow::go_to_page_close(){
    ui->stackedWidget->setCurrentIndex(2);
}
void MainWindow::go_to_page_userin(){
    ui->stackedWidget->setCurrentIndex(4);
    this->trainer_user();

    ui->userin_save_Btn_2->setEnabled(true);
    for (int i = 0; i < lst_trainer_name.size(); i++){      //트레이너 이름 콤보박스에 넣기
        ui->tr_combobox->addItem(lst_trainer_name[i]);
    }
}

void MainWindow::setDB(){       // DB 접속 설정
    this->db.setHostName("10.10.20.123");
    this->db.setPort(3306);       // 이것만 정수
    this->db.setUserName("pipipi");
    this->db.setPassword("1234");
    this->db.setDatabaseName("HEALTH");
}

void MainWindow::deadline_db(){   //마감버튼 눌렀을때  - 진영
    if(db.open()) {
        QSqlQuery query(this->db);
        QString sql = "SELECT USER_ID, TRAINER_ID, PT_DAY, STATUS FROM PT_SCHEDULE";

        if(!query.exec(sql)) {
            db.close();
            return;
        }

        while (query.next()){
            db_pt_data tmp;
            tmp.pt_user_id = query.value(0).toInt();
            tmp.pt_tr_id = query.value(1).toInt();
            tmp.pt_day = query.value(2).toDateTime();
            tmp.pt_day_str = tmp.pt_day.toString("yyyy-MM-dd");     //이것도 년 월 일까지만 담기
            tmp.status = query.value(3).toInt();
            this->pt_list.append(tmp);
            qDebug() <<"pt_list: " << tmp.pt_day.date().toString("yyyy-MM-dd");
        }

        QString today_time = QDateTime::currentDateTime().toString("yyyy-MM-dd");   //년 월 일까지만 담기
        qDebug() << "현재 오늘 시간: " << today_time;

        for (int i = 0; i < this->pt_list.size(); i++){
            QString sql1 = QString("UPDATE USER_INFO JOIN PT_SCHEDULE ON USER_INFO.U_ID = PT_SCHEDULE.USER_ID AND DATE(PT_SCHEDULE.PT_DAY) = CURDATE() SET USER_INFO.U_PT_CNT = USER_INFO.U_PT_CNT - 1 WHERE PT_SCHEDULE.STATUS = 3");
            QString sql = QString("UPDATE PT_SCHEDULE SET STATUS = 0 WHERE DATE(PT_DAY) = CURDATE()"); //now()는 시간까지 비교함 그래서 CURDATE로 비교

            if (!query.exec(sql1)) {
                qDebug() << "error";
            }
            if (!query.exec(sql)) {
                qDebug() << "error";
            }
        }
    }
    db.close();
    QMessageBox::information(this,"안내","마감되었습니다.");
    this->close();

}

void MainWindow::get_user_info(){ //유저 정보 로드
    if(db.open()){
        QSqlQuery query(this->db);
        QString query_user = "SELECT * FROM USER_INFO";

        if(!query.exec(query_user)) {
            db.close();
            return;
        }

        this->lst_user_info.clear();  //

        while (query.next()){
            std_user_info tmp;
            tmp.user_id = query.value(0).toInt();
            tmp.name = query.value(1).toString();
            tmp.birth = query.value(2).toString();
            tmp.phonenum = query.value(3).toString();
            tmp.trainer_id = query.value(4).toInt();
            tmp.user_pt_cnt = query.value(5).toInt();
            this->lst_user_info.append(tmp);
        }
        qDebug() << "불러온 회원 수: " << lst_user_info.size();
    }
}

void MainWindow::pt_click_num(const QDate &selectedDate){
    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        qDebug() << "[show_my_exercise] 기본 커넥션이 없습니다.";
        return;
    }
    QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");
    if (!db.isOpen()) {
        qDebug() << "[show_my_exercise] DB가 닫혀 있습니다. 재연결 시도...";
        if (!db.open()) {
            qDebug() << "[show_my_exercise] DB 재연결 실패";
            return;
        }
    }

    this->dateString = selectedDate.toString("yyyy-MM-dd");
    qDebug() << dateString;
}

void MainWindow::routine_write_db(int user_id){    //쓴다음에 db에 보내주는 거
    if(db.open()) {
        QSqlQuery query(this->db);
        this->routine_user.clear(); //쓰기전에 한번 씻궈주기
        QString routine1 = ui->lineEdit_1->text();
        QString routine2 = ui->lineEdit_2->text();
        QString routine3 = ui->lineEdit_3->text();
        QString routine4 = ui->lineEdit_4->text();
        QString routine5 = ui->lineEdit_5->text();
        QString routine6 = ui->lineEdit_6->text();
        this->routine_user.append(QString::number(this->user_id));
        qDebug() << user_id;
        this->routine_user.append(routine1);
        this->routine_user.append(routine2);
        this->routine_user.append(routine3);
        this->routine_user.append(routine4);
        this->routine_user.append(routine5);
        this->routine_user.append(routine6);
        QString sql = QString("INSERT INTO USER_EXERCISE(U_ID,EXERCISE1,EXERCISE2,EXERCISE3,EXERCISE4,EXERCISE5,EXERCISE6,EXERCISE_DATE) VALUES (\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\",\"%7\",\"%8\")").arg(routine_user[0],routine_user[1],routine_user[2],routine_user[3],routine_user[4],routine_user[5],routine_user[6], this->dateString);
        qDebug() << "INSERT sql: " << sql;
        ui->lineEdit_1->setText("");
        ui->lineEdit_2->setText("");
        ui->lineEdit_3->setText("");
        ui->lineEdit_4->setText("");
        ui->lineEdit_5->setText("");
        ui->lineEdit_6->setText("");
        qDebug() << routine_user;
        if (!query.exec(sql)) {
            qDebug() << "error";
        }
    }
    db.close();
}

// 기능
void MainWindow::click_calendar(const QDate &date){ //서현 추가
    select_time = date.toString("yyyy-MM-dd"); //쿼리에 쓰는 양식으로 서식 지정함
    qDebug() << "선택한 날짜 :" << select_time;
    clear_lineEdits();
    routine_road_db();
}

void MainWindow::clear_lineEdits(){     //서현 추가 라인 에딧 클리어하기
    ui->lineEdit_1->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->lineEdit_4->clear();
    ui->lineEdit_5->clear();
    ui->lineEdit_6->clear();
}

void MainWindow::routine_road_db(){    //운동 숙제 정보 읽어오기
    int U_ID = user_id;
    QList<QLineEdit*> lineEdits = {
        ui->lineEdit_1, ui->lineEdit_2, ui->lineEdit_3,
        ui->lineEdit_4, ui->lineEdit_5, ui->lineEdit_6
    };
    for(int i = 0; i<lineEdits.size();i++){
        QFont font = lineEdits[i]->font();
        font.setBold(false);
        font.setStrikeOut(false); //가운데 선
        lineEdits[i]->setFont(font); //출력
    }
    if(db.open()) {
        QSqlQuery query(this->db);
        QString sql_exercise = QString(
                                   "SELECT EXERCISE1, EXERCISE2, EXERCISE3, EXERCISE4, EXERCISE5, EXERCISE6, "
                                   "EXERCISE1_CHECK, EXERCISE2_CHECK, EXERCISE3_CHECK, EXERCISE4_CHECK, EXERCISE5_CHECK, EXERCISE6_CHECK "
                                   "FROM USER_EXERCISE "
                                   "WHERE U_ID = %1 AND DATE(EXERCISE_DATE) = '%2' "
                                   "LIMIT 1"
                                   ).arg(U_ID).arg(select_time);
        if(!query.exec(sql_exercise)) {
            db.close();
            return;
        }

        qDebug() << "uid: " << U_ID << "query: " << sql_exercise;

        if (query.next()) {

            for (int i=0; i<lineEdits.size(); i++){
                QString tmp = query.value(i).toString();
                lineEdits[i]->setText(tmp);

                bool tmp_bool = query.value(i + lineEdits.size()).toBool();
                if(tmp_bool) {
                    QFont font = lineEdits[i]->font();
                    font.setBold(true);//볼드체
                    font.setStrikeOut(true); //가운데 선
                    lineEdits[i]->setFont(font);
                }
            }
        }
    }
}

void MainWindow::trainer_info(){        //트레이너 이름 담는 함수
    if(db.open()) {
        QSqlQuery query(this->db);
        QString sql = "SELECT NAME, ID FROM TRAINER_INFO"; //**서현 수정
        if(!query.exec(sql)) {
            db.close();
            return;
        }
        this->lst_trainer_name.clear(); //리스트 담기전에 초기화
        while (query.next()) {
            this->lst_trainer_name.append(query.value(0).toString());
            this->lst_trainer_id.append(query.value(1).toInt()); //**서현 추가
        }
        for(int i=0; i<lst_trainer_name.size();i++){
            trainerMap[lst_trainer_name[i]]=lst_trainer_id[i];
        }
        ui->tr_Btn1->setText(lst_trainer_name.value(0));
        ui->tr_Btn2->setText(lst_trainer_name.value(1));
        ui->tr_Btn3->setText(lst_trainer_name.value(2));
        ui->tr_Btn4->setText(lst_trainer_name.value(3));
        ui->tr_Btn5->setText(lst_trainer_name.value(4));
        ui->tr_Btn6->setText(lst_trainer_name.value(5));
    }
}

void MainWindow::trBtn_handling(){      //********************0605

    //버튼 비활성화 시키기
    QList<QPushButton*> buttons = {
        ui->tr_Btn1, ui->tr_Btn2, ui->tr_Btn3,
        ui->tr_Btn4, ui->tr_Btn5, ui->tr_Btn6
    };

    for (QPushButton* btn : buttons) {
        btn->disconnect(); //먼저 디스커넥트하기
        connect(btn, &QPushButton::clicked, this, [=]() {
            if (currentActiveTrainerBtn == btn) {
                // 같은 버튼을 다시 눌렀으면 전체 활성화
                for (QPushButton* b : buttons)
                    b->setEnabled(true);
                currentActiveTrainerBtn = nullptr;
            } else {
                // 다른 버튼 누른 경우 나머지 비활성화
                for (QPushButton* b : buttons)
                    b->setEnabled(b == btn);
                currentActiveTrainerBtn = btn;
            }
        });
    }

    for(int i = 0; i < buttons.size(); i++){
        QPushButton* btn = buttons[i];
        connect(btn,&QPushButton::clicked, this, [this, btn](){
            this->trainer_name = btn->text(); //버튼에 쓰인 트레이너명을 변수에 담는다

            this->trainer_id = trainerMap.value(this->trainer_name);
            qDebug() << "선택된 트레이너 번호" <<trainer_id;
            set_user_name_Btn();
        });
    }
}


void MainWindow::clear_user_name_Btn() {
    QLayoutItem* item;
    while ((item = ui->gridLayout_namewidget->takeAt(0)) != nullptr) {
        QWidget* widget = item->widget();
        if (widget) {
            widget->setParent(nullptr);  // 부모 제거
            delete widget;               // 메모리 해제
        }
        delete item;
    }

    lst_username_btns.clear();  // 리스트도 초기화
}

void MainWindow::set_user_name_Btn() {
    clear_user_name_Btn(); //먼저 기존 버튼들 삭제함
    int row = 0; //버튼 한 줄씩 추가할 떄 사용할 행 번호~

    for (int i = 0; i < lst_user_info.size(); i++) {
        if (lst_user_info[i].trainer_id == trainer_id) { //회원정보의 담당 트레이너가 선택된 트레이너 아이디와 같은 경우
            QPushButton* btn = new QPushButton(lst_user_info[i].name); //버튼에 유저 이름을 적은 객체를 생성
            lst_username_btns.append(btn); //리스트에 버튼을 추가! (얘를 기준으로 나중에 활성화/비활성화 한다.)
            ui->gridLayout_namewidget->addWidget(btn, row++, 0); //그리드에 추가

            connect(btn, &QPushButton::clicked, this, [this, btn]() {  //람다로 클릭시 실행할 동작을 연결하기
                if (currentActiveUserBtn == btn) {   //현재 활성화 된 버튼이 지금 버튼일 때 (이미눌렀는디 또 눌렀따)
                    clear_lineEdits(); //라인 에딧들 지워줌  UI입력창 초기화
                    for (int j = 0; j < lst_username_btns.size(); j++)
                        lst_username_btns[j]->setEnabled(true); //모든 회원 이름 버튼 활성화
                    currentActiveUserBtn = nullptr; //그리고 현재 활성화된 버튼 상태를 없애주기
                } else {
                    clear_lineEdits(); //라인 에딧 지우기 UI입력창 초기화
                    for (int j = 0; j < lst_username_btns.size(); j++)
                        lst_username_btns[j]->setEnabled(lst_username_btns[j] == btn); //지금 클릭된 버튼만 활성화 나머지 비활성화
                    currentActiveUserBtn = btn;             // 현재 활성화된 버튼을 이 버튼으로 덮어쓰기

                }

                for (int i = 0; i < lst_user_info.size(); i++) {  // 버튼에 적힌 이름을 기준으로 user_id 값을 찾아낸다
                    if (lst_user_info[i].name == btn->text()) {
                        user_id = lst_user_info[i].user_id;
                        break;
                    }
                }
            });
        }
    }
}


void MainWindow::trainer_user() {       //회원정보 출력창 함수
    if(db.open()) {
        QSqlQuery query(this->db);
        QString sql = "SELECT * FROM USER_INFO";

        if(!query.exec(sql)) {
            db.close();
            return;
        }

        int columnCount = query.record().count();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(columnCount);

        int row = 0;
        QStringList headers;
        headers << "회원 ID" << "이름" << "생년월일" << "전화번호" << "담당트레이너 번호" << "pt남은 횟수";    //이름 원하는걸로 바꿈 수고
        ui->tableWidget->setHorizontalHeaderLabels(headers);    //헤더 출력시키기}

        this->todos.clear();
        while(query.next()) {
            std_user_info tmp;
            tmp.user_id = query.value(0).toInt();
            tmp.name = query.value(1).toString();
            tmp.birth = query.value(2).toString();
            tmp.phonenum = query.value(3).toString();
            tmp.trainer_id = query.value(4).toInt();
            tmp.user_pt_cnt = query.value(5).toInt();

            this->todos.append(tmp);

            ui->tableWidget->insertRow(row);

            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(tmp.user_id)));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(tmp.name));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(tmp.birth));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(tmp.phonenum));
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(tmp.trainer_id)));
            ui->tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(tmp.user_pt_cnt)));
            row++;
        }
        // ui->tableWidget->resizeColumnsToContents();     //열 너비 조정하는 거
        db.close();
    }
}

void MainWindow::set_user_info_Btn() {      //회원정보 확인/수정 함수
    if (db.open()) {
        QSqlQuery query(this->db);
        int row = ui->tableWidget->currentRow();

        if (row < 0) {
            qDebug() << "선택된 행이 없음";
            db.close();
            return;
        }

        // 테이블에서 값 싹 다 읽음
        int user_id = ui->tableWidget->item(row, 0)->text().toInt();  // 기존 ID
        QString name = ui->tableWidget->item(row, 1)->text();
        QString birth = ui->tableWidget->item(row, 2)->text();
        QString phone = ui->tableWidget->item(row, 3)->text();
        int trainer = ui->tableWidget->item(row, 4)->text().toInt();
        int ptcnt = ui->tableWidget->item(row, 5)->text().toInt();

        QString sql = "UPDATE USER_INFO SET U_NAME = :name, U_BIRTH = :birth, U_PHONENUM = :phone, U_TRAINER = :trainer, U_PT_CNT = :ptcnt WHERE U_ID = :id";
        query.prepare(sql);
        qDebug() << "sql: " << sql;
        // bindValue(:변수명 자리와 값을 바꿈요)
        query.bindValue(":id", user_id);
        query.bindValue(":name", name);
        query.bindValue(":birth", birth);
        query.bindValue(":phone", phone);
        query.bindValue(":trainer", trainer);
        query.bindValue(":ptcnt", ptcnt);

        if (!query.exec()) {
            qDebug() << "업데이트 실패";
        } else {
            qDebug() << "업데이트 성공";
        }
        db.close();


    }
}

void MainWindow::new_user_insert() {
    QString user_name = ui->name_lineEdit->text();
    QString user_bt   = ui->bt_lineEdit->text();
    QString user_ph   = ui->ph_lineEdit->text();
    QString user_tr   = ui->tr_combobox->currentText(); //담당
    int user_tr_key = trainerMap[user_tr];
    int user_pt       = ui->pt_spinbox->text().toInt();
    qDebug() << user_tr_key;
    QMessageBox::information(this, "알림", "회원 추가 되었습니다");
    ui->stackedWidget->setCurrentIndex(0);

    if(db.open()) {
        qDebug() << "DB 열림";
        QSqlQuery query(this->db);
        QString sql = QString("INSERT INTO USER_INFO(U_NAME, U_BIRTH, U_PHONENUM, U_TRAINER, U_PT_CNT) "
                              "VALUES (:name, :birth, :phone, :trainer, :ptcnt)");
        query.prepare(sql);  // 반드시 prepare 해야 바인딩 작동함

        query.bindValue(":name", user_name);
        query.bindValue(":birth", user_bt);
        query.bindValue(":phone", user_ph);
        query.bindValue(":trainer", user_tr_key);
        query.bindValue(":ptcnt", user_pt);

        qDebug() << "sql: " << sql;
        if (!query.exec()) {
            qDebug() << "error";
        }
    }
    db.close();

    ui->userin_save_Btn_2->setDisabled(true);
}


MainWindow::~MainWindow()
{

    delete ui;
}
