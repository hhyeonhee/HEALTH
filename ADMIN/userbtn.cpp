#include "userbtn.h"
#include "ui_userbtn.h"

Userbtn::Userbtn(QWidget *parent, QString name)
    : QWidget(parent)
    , ui(new Ui::userbtn)
{
    ui->setupUi(this);
    ui->UserNameBtn->setText(name); //이름을 적어주는 ~
}

Userbtn::~Userbtn()
{
    delete ui;
}
