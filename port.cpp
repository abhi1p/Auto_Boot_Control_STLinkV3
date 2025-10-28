#include "port.h"
#include "ui_port.h"

Port::Port(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::port)
{
    ui->setupUi(this);
}

Port::~Port()
{
    delete ui;
}
