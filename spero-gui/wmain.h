#ifndef WMAIN_H
#define WMAIN_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class wMain;
}

class wMain : public QWidget
{
    Q_OBJECT

public:
    explicit wMain(QWidget *parent = nullptr);
    ~wMain();
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    Ui::wMain *ui;
};

#endif // WMAIN_H
