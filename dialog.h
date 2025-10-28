#ifndef DIALOG_H
#define DIALOG_H

#include "qcustomplot.h"
#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    QCustomPlot* plotInstance();
    QTextEdit* textEditInstance();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
