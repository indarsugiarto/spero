#include "sperogui.h"
#include "ui_sperogui.h"
#include <qdebug.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>
#include <QLoggingCategory>
#include <math.h>

speroGUI::speroGUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::speroGUI),
    pubConnection(false),
    subConnection(false)
{
    ui->setupUi(this);
    // Beberapa elemen harus diset readonly
    ui->cbCharging->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbCharging->setFocusPolicy(Qt::NoFocus);
    ui->tablet->setAttribute(Qt::WA_TransparentForMouseEvents); ui->tablet->setFocusPolicy(Qt::NoFocus);
    ui->cbConnected->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbConnected->setFocusPolicy(Qt::NoFocus);
    ui->cbDisinfectant->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbDisinfectant->setFocusPolicy(Qt::NoFocus);
    ui->cbJoystick->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbJoystick->setFocusPolicy(Qt::NoFocus);
    ui->cbCharging->setAttribute(Qt::WA_TransparentForMouseEvents); ui->cbCharging->setFocusPolicy(Qt::NoFocus);
    ui->locked->setAttribute(Qt::WA_TransparentForMouseEvents); ui->locked->setFocusPolicy(Qt::NoFocus);
    ui->s1->setReadOnly(true); ui->s2->setReadOnly(true); ui->s3->setReadOnly(true); ui->s4->setReadOnly(true);
    ui->s5->setReadOnly(true); ui->s6->setReadOnly(true); ui->s7->setReadOnly(true); ui->s8->setReadOnly(true);

    // TODO: omnicam dan frontcam belum terintegrasi, maka GUI-nya kita kecilkan saja
    // ukuran idealnya: 1180 x 840
    // kalau kontrolnya saja: 470 x 840
    this->setGeometry(0,0,470,840);

    /* Pertama, coba buka file konfigurasi. Jika file ditemukan, baca dari file tersebut. Jika tidak, bikin file baru.*/
    getConfig();

    // berikut berhubungan dengan mqtt
    m_publisher = new QMqttClient(this);
    qDebug() << QString("Will use server at %1:1883 for publishing").arg(robotIP);
    m_publisher->setHostname(robotIP);
    m_publisher->setPort(1883);
    connect(m_publisher, &QMqttClient::stateChanged, this, &speroGUI::updateLogStateChange);
    connect(m_publisher, &QMqttClient::disconnected, this, &speroGUI::brokerDisconnected);
    connect(m_publisher, &QMqttClient::connected, this, &speroGUI::pubConnected);
    connect(m_publisher, &QMqttClient::disconnected, this, &speroGUI::pubDisconnected);
    m_subscriber = new QMqttClient(this);
    qDebug() << QString("Will use server at %1:1883 for subscription").arg(robotIP);
    m_subscriber->setHostname(robotIP);
    m_subscriber->setPort(1883);
    connect(m_subscriber, &QMqttClient::stateChanged, this, &speroGUI::updateLogStateChange);
    connect(m_subscriber, &QMqttClient::disconnected, this, &speroGUI::brokerDisconnected);
    connect(m_subscriber, &QMqttClient::connected, this, &speroGUI::subConnected);
    connect(m_subscriber, &QMqttClient::disconnected, this, &speroGUI::subDisconnected);
    connect(m_subscriber, &QMqttClient::messageReceived, this, &speroGUI::newMsg);
    qDebug() << QString("Will now connect to the server...");
    m_publisher->connectToHost();
    m_subscriber->connectToHost();

    // lalu setup pinger
    m_ping = new QTimer(this);
    m_ping->setInterval(ROBOT_PING_INTERVAL_MS);    //mungkin cukup tiap 1000ms kirim ping ke robot
    connect(m_ping, SIGNAL(timeout()), this, SLOT(sendPing()));

    // berikut berhubungan dengan joystick
    QLoggingCategory::setFilterRules(QStringLiteral("qt.gamepad.debug=true"));

    auto gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty()) {
        qDebug() << "Did not find any connected gamepads";
        return;
    }

    m_gamepad = new QGamepad(*gamepads.begin(), this);
    connect(m_gamepad, SIGNAL(buttonStartChanged(bool)), this, SLOT(btnStartPressed(bool)));
    connect(m_gamepad, SIGNAL(axisLeftXChanged(double)), this, SLOT(getLX(double)));    //strive
    connect(m_gamepad, SIGNAL(axisLeftYChanged(double)), this, SLOT(getLY(double)));    //fwd-back
    connect(m_gamepad, SIGNAL(axisRightXChanged(double)), this, SLOT(getRX(double)));   //rotate
    connect(m_gamepad, SIGNAL(axisRightYChanged(double)), this, SLOT(getRY(double)));   //nothing
    connect(m_gamepad, SIGNAL(buttonSelectChanged(bool)), this, SLOT(btnSelectPressed(bool)));
    connect(m_gamepad, SIGNAL(buttonGuideChanged(bool)), this, SLOT(btnGuidePressed()));


    // Konfigurasi SIGNAL-SLOT GUI umum:
    connect(ui->pbStop, SIGNAL(clicked()), this, SLOT(pbStopClicked()));
    connect(ui->hSlider, SIGNAL(valueChanged(int)), this, SLOT(hSliderChanged(int)));
    connect(ui->vSlider, SIGNAL(valueChanged(int)), this, SLOT(vSliderChanged(int)));
    connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(dialChanged(int)));
    connect(ui->pbSimpan, SIGNAL(clicked()), this, SLOT(pbSimpanClicked()));
    connect(ui->pbHalt, SIGNAL(clicked()), this, SLOT(pbHaltClicked()));
}

speroGUI::~speroGUI()
{
    delete m_gamepad;
    delete m_publisher;
    delete m_subscriber;
    delete m_ping;
    delete ui;
}

void speroGUI::pubConnected()
{
    pubConnection = true;
}

void speroGUI::pubDisconnected()
{
    pubConnection = false;
}

void speroGUI::subDisconnected()
{
    subConnection = false;
    ui->cbConnected->setChecked(false);
}

void speroGUI::subConnected()
{
    subConnection = true;
    ui->cbConnected->setChecked(true);
    auto subscription = m_subscriber->subscribe(QString(TOPIC_STATUS));
    if (!subscription) {
            QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
            return;
    }
}

/* Kirim sinyal secara periodik ke robot */
void speroGUI::sendPing()
{

}

void speroGUI::newMsg(const QByteArray &message, const QMqttTopicName &topic)
{
    if(topic.name().contains(TOPIC_STATUS)){
        qDebug() << "[INFO] Menerima pesan MQTT dari robot" << "\n";
    }
}

void speroGUI::pbHaltClicked()
{
    int ret = QMessageBox::warning(this, "PERINGATAN", "Anda yakin akan menghentikan operasional robot SPERO?",
                         QMessageBox::Yes | QMessageBox::Cancel);
    if(ret==QMessageBox::Yes){
        qDebug () << "Ada request untuk shutdown SPERO";
        QString cmd = QString(BRAKE_CMD);
        sendCmd(cmd);
        // TODO: then wait until SPERO is completely shutted down (via ping?)
        this->close();
    }
}

void speroGUI::updateLogStateChange()
{
    const QString content1 = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_publisher->state())
                    + QLatin1Char('\n');
    const QString content2 = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_subscriber->state())
                    + QLatin1Char('\n');
    qDebug() << (content1);
    qDebug() << (content2);
    ui->console->appendPlainText(content1);
    ui->console->appendPlainText(content2);
}

void speroGUI::brokerDisconnected()
{
    QString msg = QString("[INFO] Terputus dari server...") + QLatin1Char('\n');;
    qDebug() << msg;
    ui->console->appendPlainText(msg);
}


void speroGUI::pbSimpanClicked()
{
    robotIP = ui->robotIP->text();
    omnicamIP = ui->omnicamIP->text();
    frontcamIP = ui->frontcamIP->text();
    if(saveConfig()==0){
        QMessageBox::information(this, "Informasi", "Penyimpanan konfigurasi berhasil dilakukan");
    } else {
        QMessageBox::critical(this, "Error", "Tidak berhasil menyimpan konfigurasi yang baru");
    }
}

int speroGUI::saveConfig()
{
    QFile file(CONFIG_FILE_NAME);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QString msg = QString("[ERROR] Tidak bisa menulis file konfigurasi");
        ui->console->insertPlainText(msg);
        qDebug() << msg;
        return -1;
    } else {
        QTextStream out(&file);
        out << "robotIP=" << robotIP << "\n";
        out << "omnicamIP=" << omnicamIP << "\n";
        out << "frontcamIP=" << frontcamIP << "\n";
        file.close();
        return 0;
    }
}

void speroGUI::getConfig()
{
    QFile file(CONFIG_FILE_NAME);
       if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
       {
           qDebug() << "[INFO] Mencoba membuat file konfigurasi baru";
           robotIP = DEFAULT_ROBOT_IP;
           omnicamIP = DEFAULT_OMNICAM_IP;
           frontcamIP = DEFAULT_FRONTCAM_IP;
           saveConfig();
       } else {
           qDebug() << "[INFO] Membaca file konfigurasi";
           QTextStream in(&file);
           while (!in.atEnd()) {
               QString line = in.readLine();
               qDebug() << line;
               QStringList list = line.split(QLatin1Char('='));
               if(list[0].contains(PARAM_NAME_ROBOT_IP)){
                   ui->robotIP->setText(list[1]);
                   robotIP = list[1];
               } else if(list[0].contains(PARAM_NAME_OMNICAM_IP)) {
                   ui->omnicamIP->setText(list[1]);
                   omnicamIP = list[1];
               } else if(list[0].contains(PARAM_NAME_FRONTCAM_IP)) {
                   ui->frontcamIP->setText(list[1]);
                   frontcamIP = list[1];
               }
            }
           file.close();
       }

}


void speroGUI::pbStopClicked()
{
    qDebug() << "Tombol Stop ditekan!";
    ui->hSlider->setValue(0);
    ui->vSlider->setValue(0);
    ui->dial->setValue(0);
}

void speroGUI::hSliderChanged(int newVal)
{
    qDebug() << QString("nilai hSlider = %1").arg(newVal);
    double x = (double)newVal / 100.0;
    lx = x;
    prepareCmd(JS_KIRI);
}

void speroGUI::vSliderChanged(int newVal)
{
    qDebug() << QString("nilai vSlider = %1").arg(newVal);
    double y = (double)newVal / 100.0;
    ly = y;
    prepareCmd(JS_KIRI);
}

void speroGUI::dialChanged(int newVal)
{
    qDebug() << QString("nilai dial = %1").arg(newVal);
    double r = (double)newVal / 100.0;
    rx = r;
    prepareCmd(JS_KANAN);
}

/*------------------- Joystick ------------------*/
void speroGUI::getRX(double val)
{
    qDebug() << QString("RX val = %1").arg(val);
    rx = val;
    prepareCmd(JS_KANAN);
}

void speroGUI::getRY(double val)
{
    ry = val;   // tidak perlu prepareCmd, karena joystick kanan hanya mendeteksi AXIS-X
}
void speroGUI::getLX(double val)
{
    qDebug() << QString("LX val = %1").arg(val);
    lx = val;
    prepareCmd(JS_KIRI);
}
void speroGUI::getLY(double val)
{
    qDebug() << QString("LY val = %1").arg(val);
    ly = val;
    prepareCmd(JS_KIRI);
}
void speroGUI::btnStartPressed(bool status)
{
    // tombol start kita gunakan untuk test base
    if(status){
        int ret = QMessageBox::question(this, "WARNING", "Akan dilakukan Test BASE. Pastikan di sekitar area sudah clear. Lanjut?",
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if(ret == QMessageBox::Yes){
            QString cmd = QString(TEST_BASE);
            sendCmd(cmd);
        }
    } else {
    }
}
void speroGUI::btnSelectPressed(bool status)
{
    // tombol select kita gunakan untuk test roda
    if(status){
        int ret = QMessageBox::question(this, "WARNING", "Akan dilakukan Test Roda. Pastikan robot dalam posisi floating. Lanjut?",
                                        QMessageBox::Yes | QMessageBox::Cancel);
        if(ret == QMessageBox::Yes){
            QString cmd = QString(TEST_RODA_CMD);
            sendCmd(cmd);
        }
    }
}
void speroGUI::btnGuidePressed(bool status)
{
    qDebug() << "Guide button is pressed!";
    if(status){
        QString cmd = QString(BRAKE_CMD);
        sendCmd(cmd);
    }
}

void speroGUI::prepareCmd(int m)
{
    /* stick KIRI untuk: majur, mundur, geser kanan, geser kiri
     * stick KANAN hanya untuk putar
     * *********************************************************/
    QString cmd;
    if(m==JS_KIRI){                     // joystick kiri dioperasikan
            if(fabs(ly)>fabs(lx)) {     // gerakan maju mundur
                    if(ly<0) {          // maka berarti maju
                        cmd = QString("%1,%2,%3")
                                .arg(HEADER).arg(MAJU).arg(fabs(ly));
                    } else {            // maka berarti mundur
                        cmd = QString("%1,%2,%3")
                                .arg(HEADER).arg(MUNDUR).arg(ly);
                    }
            } else {                    // gerakan geser kanan/kiri
                    if(lx<0) {          // geser kiri
                            cmd = QString("%1,%2,%3")
                                    .arg(HEADER).arg(GESER_KIRI).arg(fabs(lx));
                    } else {            // geser kanan
                            cmd = QString("%1,%2,%3")
                                    .arg(HEADER).arg(GESER_KANAN).arg(lx);
                    }
            }
    } else {                            // joystick kanan dioperaasikan
            if(rx<0){                   // putar kiri
                    cmd = QString("%1,%2,%3")
                            .arg(HEADER).arg(PUTAR_KIRI).arg(fabs(rx));
            } else {
                    cmd = QString("%1,%2,%3")
                            .arg(HEADER).arg(PUTAR_KANAN).arg(rx);
            }
    }
    sendCmd(cmd);
}

void speroGUI::sendCmd(QString cmd)
{
    // modifikasi dari on_buttonPublish_clicked()
    if (m_publisher->publish(QString(TOPIC_JS), cmd.toUtf8()) == -1)
        qDebug() << QString("[INFO] Error, could not send the command!");
}

void speroGUI::sendGreetings()
{
    qDebug() << QString("[INFO] Kirim greetings ke server...");
    sendCmd(QString("Halo controller..."));
}

