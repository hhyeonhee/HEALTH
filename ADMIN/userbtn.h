#ifndef USERBTN_H
#define USERBTN_H

#include <QWidget>

namespace Ui {
class userbtn;
}

class Userbtn : public QWidget
{
    Q_OBJECT

public:
    explicit Userbtn(QWidget *parent = nullptr, QString name ="");
    ~Userbtn();

private:
    Ui::userbtn *ui;
};

#endif // USERBTN_H
