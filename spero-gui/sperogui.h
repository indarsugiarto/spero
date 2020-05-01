#ifndef SPEROGUI_H
#define SPEROGUI_H

#include <QWidget>

class speroGui : public QWidget
{
    Q_OBJECT

public:
    speroGui(QWidget *parent = nullptr);
    ~speroGui();
};
#endif // SPEROGUI_H
