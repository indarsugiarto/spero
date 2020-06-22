#include "wmain.h"
#include "ui_wmain.h"
#include <QMessageBox>
#include <QKeySequence>

wMain::wMain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wMain)
{
    ui->setupUi(this);
    this->setWindowState(Qt::WindowFullScreen);
}

wMain::~wMain()
{
    delete ui;
}

void wMain::keyPressEvent(QKeyEvent *event)
{
    QKeySequence quit = QKeySequence("Ctrl+Q");
    if(event->key() == Qt::Key_F2){
        //QMessageBox::information(this, "COba", "Deteksi F2");
        this->ui->tabWidget->setCurrentIndex(0);
    }
    else if(event->key() == Qt::Key_F3){
        this->ui->tabWidget->setCurrentIndex(1);
    }
    // jika tombol "Tutup" yaitu Ctrl-T ditekan, kita keluar
    else if(event->matches(QKeySequence::AddTab)) {
    //else if(event->matches(QKeySequence::Quit)) {
    //else if(event->key() == Qt::Key_Q){
        int tanya = QMessageBox::question(this, "Keluar", "Anda Yakin");
        if(tanya==QMessageBox::Yes) this->close();
    }
}
