#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDateTime>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , server(new QTcpServer(this))
{
    ui->setupUi(this);
    this->db_open();
    ui->Btn_send_chat->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(0);

    //페이징함수
    connect(ui->Btn_go_chat,&QPushButton::clicked,this,&MainWindow::turn_page_chat);
    connect(ui->Btn_pt_check,&QPushButton::clicked,this,&MainWindow::turn_page_pt);

    // 버튼 클릭 → 메시지 브로드캐스트
    connect(ui->Btn_send_chat, &QPushButton::clicked,
            this, &MainWindow::onPushButtonClicked);

    // 새 클라이언트 접속
    connect(server, &QTcpServer::newConnection,
            this,   &MainWindow::onNewConnection);

    // 서버 시작
    const quint16 port = 12345;
    if (!server->listen(QHostAddress::Any, port)) {
        appendMessage(ui->serverChatLayout,
                      "[오류] 서버 시작 실패: " + server->errorString());
    } else {
        appendMessage(ui->serverChatLayout,
                      QString("[시스템] 서버 실행 중 - 포트 %1").arg(port));
        ui->Btn_send_chat->setEnabled(true);
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void  MainWindow::db_open(){
    QSqlDatabase db;
    if (QSqlDatabase::contains("pt_connection")) {
        db = QSqlDatabase::database("pt_connection");
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL", "pt_connection");
        db.setHostName("10.10.20.123");
        db.setDatabaseName("HEALTH");
        db.setUserName("pipipi");
        db.setPassword("1234");
        db.setPort(3306);
    }

    if (!db.isOpen() && !db.open()) {
        qDebug() << "❌ DB 열기 실패:" << db.lastError().text();
        return;
    }
}

void MainWindow::turn_page_chat(){
    ui->stackedWidget->setCurrentIndex(0);

}

void MainWindow::turn_page_pt(){
    ui->stackedWidget->setCurrentIndex(1);
    show_pt_list();

}

// ... (MainWindow.cpp 상단 include 및 다른 함수들) ...

void MainWindow::show_pt_list()
{
    // 1. 기존 PT 변경 위젯 제거 (이 부분은 그대로 유지하거나 살짝 수정)
    // 이전 show_pt_list() 호출로 인해 남아있는 위젯들을 정리합니다.
    // 이제 pt_change 위젯 자체에서 deleteLater()를 호출하지 않으므로,
    // 리스트에 남아있는 위젯들은 여기서 삭제 스케줄을 잡아줘야 합니다.
    for (pt_change* widget : pt_change_list) {
        ui->verticalLayout->removeWidget(widget);
        // setParent(nullptr)는 불필요
        widget->deleteLater(); // 이전 호출로 생긴 위젯들 삭제 예약
    }
    pt_change_list.clear(); // 리스트를 비워서 dangling pointer 문제 방지

    // 2. 기존 "📌 PT 변경 요청" 메시지만 제거 (이 부분은 그대로)
    QList<QLabel*> labelsToDelete;
    for (int i = 0; i < ui->serverChatLayout->count(); ++i) {
        QLayoutItem* item = ui->serverChatLayout->itemAt(i);
        if (QLabel* label = qobject_cast<QLabel*>(item->widget())) {
            if (label->text().contains("📌 PT 변경 요청:")) {

                labelsToDelete.append(label);
            }
        }
    }
    for (QLabel* label : labelsToDelete) {
        ui->serverChatLayout->removeWidget(label);
        delete label;
    }

    // 3. DB 연결 및 새 위젯 생성
    QSqlDatabase db = QSqlDatabase::database("pt_connection");
    if (!db.isOpen() && !db.open()) {
        qDebug() << "❌ DB 열기 실패:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    QString sql = R"(
        SELECT
            PT.ID,
            PT.PT_DAY,
            PT.PT_TIME,
            PT.STATUS,
            UI.U_NAME AS USER_NAME,
            TI.NAME AS TRAINER_NAME
        FROM
            PT_SCHEDULE PT
        JOIN
            USER_INFO UI ON PT.USER_ID = UI.U_ID
        JOIN
            TRAINER_INFO TI ON PT.TRAINER_ID = TI.ID
        WHERE
            PT.STATUS = 1;
    )";
//여기야ㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑㅑ
    if (!query.exec(sql)) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int id = query.value("ID").toInt();
        QString user_name = query.value("USER_NAME").toString();
        QString trainer_name = query.value("TRAINER_NAME").toString();
        QString pt_day = query.value("PT_DAY").toDateTime().toString("yyyy-MM-dd");
        QString pt_time = query.value("PT_TIME").toString();
        int status = query.value("STATUS").toInt();

        QString pt_option;

        if (status == 1) {
            pt_option = QString("NO: %1 | 회원: %2 | 담당트레이너: %3 | 희망일자: %4 | 희망시간: %5 시")
                            .arg(id).arg(user_name).arg(trainer_name).arg(pt_day).arg(pt_time);
        }

        pt_change* widget = new pt_change(ui->frame_2, pt_option, id);

        // ✨ 여기서 중요! pt_change 위젯의 finished 시그널을 MainWindow의 슬롯에 연결! ✨
        connect(widget, &pt_change::finished, this, &MainWindow::handlePtChangeFinished);

        pt_change_list.append(widget);
        ui->verticalLayout->addWidget(widget);

        // PT 변경 요청 메시지 출력 (중복 없이)
        appendMessage(ui->serverChatLayout, QString("📌 PT 변경 요청: %1").arg(user_name));
    }
    db.close(); // DB 연결 닫기
}

// ✨ 새로 추가된 슬롯 함수! ✨
// pt_change 위젯이 finished 시그널을 보낼 때 호출됩니다.
void MainWindow::handlePtChangeFinished(pt_change* widget_to_delete)
{
    // ✨ 이제 schedule_id에 직접 접근하는 대신 getScheduleId() 함수를 사용하자! ✨
    qDebug() << "Widget finished signal received for ID:" << widget_to_delete->getScheduleId(); // 디버깅용

    // pt_change_list에서 해당 위젯 포인터를 찾아서 제거!
    pt_change_list.removeOne(widget_to_delete);

    // 레이아웃에서도 제거!
    ui->verticalLayout->removeWidget(widget_to_delete);

    // 이제 이 위젯을 안전하게 삭제 예약!
    widget_to_delete->deleteLater();

    qDebug() << "Widget removed from list and scheduled for deletion."; // 디버깅용
}



// ... (나머지 MainWindow.cpp 내용) ...


void MainWindow::onNewConnection() {
    // pending 연결을 socket으로 꺼내고 리스트에 보관
    QTcpSocket *sock = server->nextPendingConnection();
    clients.append(sock);
    appendMessage(ui->serverChatLayout,
                  "[접속] " + sock->peerAddress().toString());

    connect(sock, &QTcpSocket::readyRead,
            this, &MainWindow::onReadyRead);
    connect(sock, &QTcpSocket::disconnected,
            this, &MainWindow::onDisconnected);
}


void MainWindow::onReadyRead() {
    QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());

    while (sock->canReadLine()) {
        QString msg = QString::fromUtf8(sock->readLine()).trimmed();

        // 아직 ID 등록 안 된 클라이언트면 이 메시지를 ID로 본다
        if (!socketMap.contains(sock)) {
            socketMap[sock] = msg;
            clientMap[msg] = sock;

            appendMessage(ui->serverChatLayout, "[접속] ID 등록됨: " + msg);
            return;
        }

        QString senderId = socketMap[sock];

        // 귓속말 처리
        if (msg.startsWith("/w ")) {
            QStringList parts = msg.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString targetId = parts[1];
                QString whisper = parts.mid(2).join(" ");
                QTcpSocket *targetSock = clientMap.value(targetId, nullptr);

                if (targetSock) {
                    QString toSend = "[💭귓속말][" + senderId + " → " + targetId + "] " + whisper + '\n';
                    targetSock->write(toSend.toUtf8());

                    QString selfMsg = "[나→" + targetId + "] " + whisper + '\n';
                    sock->write(selfMsg.toUtf8());
                } else {
                    sock->write("[서버] 해당 대상이 없습니다.\n");
                }
            }
            return;
        }

        // 일반 메시지 처리
        QString fullMsg = "[" + senderId + "] " + msg;
        appendMessage(ui->serverChatLayout, fullMsg);

        if (senderId.startsWith("trainer_")) {
            for (QTcpSocket *c : clients) {
                c->write(fullMsg.toUtf8() + '\n');
            }
        } else if (senderId.startsWith("user_")) {
            for (QTcpSocket *c : clients) {
                QString receiverId = socketMap.value(c);
                if (receiverId.startsWith("trainer_") || receiverId == "server") {
                    c->write(fullMsg.toUtf8() + '\n');
                }
            }
        }
    }
}


void MainWindow::onDisconnected() {
    QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
    QString id = socketMap.value(sock, "알수없음");
    appendMessage(ui->serverChatLayout, "[종료] " + id);

    clientMap.remove(id);
    socketMap.remove(sock);
    clients.removeAll(sock);
    sock->deleteLater();
}


void MainWindow::onPushButtonClicked() {
    QString msg = "[📢공지]"+ui->lineEdit->text().trimmed();
    if (msg.isEmpty()) return;

    QByteArray data = msg.toUtf8() + '\n';
    // 1) 서버 UI 왼쪽(user)에도 추가
    appendMessage(ui->serverChatLayout, msg);
    // 2) 연결된 모든 클라이언트로 전송
    for (QTcpSocket *c : clients) {
        c->write(data);
    }
    ui->lineEdit->clear();
}

void MainWindow::appendMessage(QVBoxLayout *layout, const QString &text) {
    QString timeStr = QDateTime::currentDateTime().toString("MM-dd HH:mm");
    QLabel *lbl = new QLabel("[" + timeStr + "] " + text, this);

    QScrollBar *vbar = ui->server_chat_scroll->verticalScrollBar();
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
