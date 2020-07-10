#ifndef BATTERYWARNING_H
#define BATTERYWARNING_H

#include <QDialog>
#include <QTimer>
#include <QHideEvent>

namespace Ui {
class BatteryWarning;
}

class BatteryWarning : public QDialog
{
    Q_OBJECT

public:
    explicit BatteryWarning(QWidget *parent = nullptr);
    ~BatteryWarning();
    void tunjukkan();

private:
    Ui::BatteryWarning *ui;
    QTimer *anim;
    bool b_anim;
    void animate();
    void hideEvent(QHideEvent *event);
};

#endif // BATTERYWARNING_H
