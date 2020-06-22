#ifndef SPEROGUI_H
#define SPEROGUI_H

#include <QWidget>
#include <QtMqtt/QMqttClient>
#include <QGamepad>
#include <QTimer>
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
    void sendPing();
    void pubConnected();
    void pubDisconnected();
    void subConnected();
    void subDisconnected();
    void newMsg(const QByteArray &message, const QMqttTopicName &topic);
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
    QMqttClient *m_publisher;
    QMqttClient *m_subscriber;
    QGamepad *m_gamepad;
    QTimer *m_ping; // dipakai untuk menjaga koneksi dengan robot
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
    bool pubConnection;
    bool subConnection;
};

#endif // SPEROGUI_H
