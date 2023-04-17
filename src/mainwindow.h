#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "h2rename.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    H2rename *h2rename;
};

#endif // MAINWINDOW_H
