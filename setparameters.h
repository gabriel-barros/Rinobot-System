#ifndef SETPARAMETERS_H
#define SETPARAMETERS_H

#include <QMainWindow>
#include <QMessageBox>
#include <vision.h>
#include <configrobots.h>

namespace Ui {
class SetParameters;
}

class SetParameters : public QMainWindow
{
    Q_OBJECT

public slots:
    void updateVisionUI(QImage img);
    void updateFPS(double val);

public:
    explicit SetParameters(QWidget *parent = 0);
    ~SetParameters();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::SetParameters *ui;
    ConfigRobots *conf;
    Vision *eye;
};

#endif // SETPARAMETERS_H