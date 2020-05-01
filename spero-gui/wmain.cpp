#include "wmain.h"
#include "ui_wmain.h"

wMain::wMain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wMain)
{
    ui->setupUi(this);
}

wMain::~wMain()
{
    delete ui;
}
