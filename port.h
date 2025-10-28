#ifndef PORT_H
#define PORT_H

#include <QWidget>

namespace Ui {
class port;
}

class Port : public QWidget
{
    Q_OBJECT

public:
    explicit Port(QWidget *parent = nullptr);
    ~Port();

private:
    Ui::port *ui;
};

#endif // PORT_H
