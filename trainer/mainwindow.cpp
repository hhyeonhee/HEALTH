#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QTcpSocket(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    // pt확인시
    this->db_open();


    connect(ui->Btn_log_out,&QPushButton::clicked,this,&MainWindow::go_to_logout);
    connect(ui->Btn_login, &QPushButton::clicked, this, &MainWindow::trainer_login);
    connect(ui->Btn_send_msg, &QPushButton::clicked,
            this, &MainWindow::onPushButtonClicked);
}

void  MainWindow::go_to_logout(){
    ui->stackedWidget->setCurrentIndex(0);
    ui->id_input->clear();
    ui->password_input->clear();
}

void MainWindow::db_open(){

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

void MainWindow::trainer_login(){

    input_id = ui->id_input->text();
    input_password = ui->password_input->text();

    // db_open();
    QSqlQuery query(QSqlDatabase::database("QMYSQL"));
    query.prepare("SELECT NAME, ID FROM TRAINER_INFO WHERE NAME = :NAME"); // :블라블라 -> 파라미터는 무엇이냐 sql에 값을 직접 넣지 않고 나중에 안전하게 바인딩
    // 파라미터를 쓰는 이유는? 사용자가 드랍테이블 하면 우째요. 내부적으로 Qt가 escape 처리 및 보안 적용 해준데요.
    // U_NAME에서 사용자가 입력한 값을 가진 행만 가져와욤
    query.bindValue(":NAME", input_id); //입력 받은 값을 U_NAME에 집어 넣어

    if (!query.exec()) {
        QMessageBox::warning(this, "쿼리 실패", query.lastError().text());
        return;
    }

    if (query.next()) { //입력한 아이디의 줄만 읽어오기 때문에 아이디가 없으면 false를 반환하기 때문에 if문에 들어가지 않습니다.!!!!
        QString ID_input = query.value("ID").toString(); //
        if (input_password == ID_input) {
            QMessageBox::information(this, "로그인 성공", "환영합니다!");
            ui->stackedWidget->setCurrentIndex(1); // 로그인 성공 하면 메인화면으로 휘리릭
            // 소켓 시그널
            connect(socket, &QTcpSocket::connected,this, &MainWindow::onConnected);
            connect(socket, &QTcpSocket::readyRead,this, &MainWindow::onReadyRead);
            connect(socket,qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred),this, &MainWindow::onErrorOccurred);
            // 서버에 접속
            socket->connectToHost("10.10.20.99", 12345);


        } else {
            QMessageBox::warning(this, "로그인 실패", "비밀번호가 일치하지 않습니다.");
            qDebug() << ID_input;
        }
    } else {

        QMessageBox::warning(this, "로그인 실패", "존재하지 않는 ID입니다.");
    }

}

// 채팅관련함수
void MainWindow::onErrorOccurred(QAbstractSocket::SocketError) {
    appendMessage(ui->verticalLayout_9,
                  "[오류] " + socket->errorString());
}
//input_id/user_chat_scroll
void MainWindow::onConnected() {
    socket->write(input_id.toUtf8() + '\n');  // 트레이너 ID 전송
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

    appendMessage(ui->verticalLayout_9, "[🙋‍나] " + msg);
    socket->write(data);
    ui->lineEdit->clear();
}

// 레이아웃찾아가는 함수인데 좀더 공부해야함 user_chat_scroll
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
    QScrollArea *area = qobject_cast<QScrollArea*>(content->parentWidget());
    if (content) {
        if (area) {
            area->verticalScrollBar()->setValue(area->verticalScrollBar()->maximum());
        }
    }

    // 3) 찾았으면 스크롤을 최하단으로
    if (area) {
        QScrollBar *vbar = area->verticalScrollBar();
        vbar->setValue(vbar->maximum());
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}
