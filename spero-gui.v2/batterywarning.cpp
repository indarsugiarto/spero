#include "batterywarning.h"
#include "ui_batterywarning.h"

BatteryWarning::BatteryWarning(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatteryWarning)
{
    ui->setupUi(this);
    anim = new QTimer(this);
    anim->setInterval(500);
    b_anim = false;
    connect(anim, &QTimer::timeout, this, &BatteryWarning::animate);
}

BatteryWarning::~BatteryWarning()
{
    delete anim;
    delete ui;
}

void BatteryWarning::animate()
{
    if(b_anim){
        ui->judul->hide();
        ui->pesan->hide();
        b_anim = false;
    } else {
        ui->judul->show();
        ui->pesan->show();
        b_anim = true;
    }
}

void BatteryWarning::tunjukkan()
{
    this->anim->start();
    this->show();
}

void BatteryWarning::hideEvent(QHideEvent *event)
{
    this->anim->stop();
}
