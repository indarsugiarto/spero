#ifndef SPEROGUI_H
#define SPEROGUI_H

#include <QWidget>
#include <QtMqtt/QMqttClient>
#include <QGamepad>
#include "sperodef.h"

namespace Ui {
class speroGUI;
}

class speroGUI : public QWidget
{
    Q_OBJECT

public:
    explicit speroGUI(QWidget *parent = nullptr);
    ~speroGUI();
public slots:
    void pbStopClicked();
    void hSliderChanged(int newVal);
    void vSliderChanged(int newVal);
    void dialChanged(int newVal);
    void pbSimpanClicked();
    void pbHaltClicked();
    // berikut ini berhubungan dengan MQTT
    void updateLogStateChange();
    void brokerDisconnected();
    // berikut berhubungan dengan joystick:
    void getRX(double val);
    void getRY(double val);
    void getLX(double val);
    void getLY(double val);
    void btnStartPressed(bool status);
    void btnSelectPressed(bool status);
    void btnGuidePressed(bool status);

private:
    // main members
    Ui::speroGUI *ui;
    QMqttClient *m_client;
    QGamepad *m_gamepad;
    // parameter konfigurasi:
    QString robotIP;
    QString omnicamIP;
    QString frontcamIP;
    // nilai-nilai dari joystick
    double rx,ry,lx,ly;
    // methods
    void getConfig();
    int saveConfig();
    void prepareCmd(int m); // m bisa digunakan untuk mendeteksi joystick yang kiri atau kanan
    void sendCmd(QString cmd);
    void sendGreetings();
};

#endif // SPEROGUI_H
