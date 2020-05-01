#ifndef WMAIN_H
#define WMAIN_H

#include <QWidget>

namespace Ui {
class wMain;
}

class wMain : public QWidget
{
    Q_OBJECT

public:
    explicit wMain(QWidget *parent = nullptr);
    ~wMain();

private:
    Ui::wMain *ui;
};

#endif // WMAIN_H
