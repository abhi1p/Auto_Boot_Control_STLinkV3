#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

QCustomPlot *Dialog::plotInstance()
{
    return ui->plot;
}

QTextEdit *Dialog::textEditInstance()
{
    return ui->textEdit;
}
