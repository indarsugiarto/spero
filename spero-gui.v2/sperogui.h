#ifndef SPEROGUI_H
#define SPEROGUI_H

#include <QWidget>
#include <QtMqtt/QMqttClient>
#include <QGamepad>
#include <QTimer>
#include "sperodef.h"
#include "batterywarning.h"
#include <QUdpSocket>

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
    void pbLepasClicked();
    void pingTimeout();
    void tray1Clicked(int state);
    void tray2Clicked(int state);
    void tray3Clicked(int state);
    void tray4Clicked(int state);
    void camClicked(int state);
    void pbLampuClicked();
    void pbChatClicked();
    void servoChanged(int newVal);
    // button-button tester
    void pbTest1Clicked();
    void pbTest2Clicked();
    void pbTest3Clicked();
    void pbTest4Clicked();
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
    void cbReleaseLockChanged(int status);

private:
    // main members
    Ui::speroGUI *ui;
    QMqttClient *m_publisher;
    QMqttClient *m_subscriber;
    QGamepad *m_gamepad;
    bool b_gamepad;
    QTimer *m_ping; // dipakai untuk menjaga koneksi dengan robot
    QTimer *m_timeout;
    QTimer *m_TabletInfo; // buat ambil data dari Tablet
    bool b_batWarning;
    QUdpSocket *udpSock;
    // parameter konfigurasi:
    QString robotIP;
    QString omnicamIP;
    QString frontcamIP;
    QString tabletIP;
    // nilai-nilai dari joystick
    double rx,ry,lx,ly;
    // methods
    void readPendingUDP();
    void askTabletBattery();
    void getConfig();
    int saveConfig();
    void prepareCmd(int m); // m bisa digunakan untuk mendeteksi joystick yang kiri atau kanan
    void sendMQTT(QString cmd);
    void sendGreetings();
    bool pubConnection;
    bool subConnection;
    BatteryWarning *batWarning;
    void showBatWarning();
    void keyPressEvent(QKeyEvent * event);
};

#endif // SPEROGUI_H
