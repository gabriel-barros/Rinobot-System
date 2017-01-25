#ifndef SOCCER_WINDOW_H
#define SOCCER_WINDOW_H

#include <QWidget>
#include "settingsdialog.h"
#include "vision.h"
#include "serial.h"

namespace Ui {
class soccer_window;
}

class soccer_window : public QWidget
{
    Q_OBJECT

public:
    explicit soccer_window(QWidget *parent = 0);
    ~soccer_window();
public slots:
    void updateSerialSettings(SettingsDialog::Settings settings);
    void updateVisionUI(QImage img);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_start_game_clicked();

private:
    Ui::soccer_window *ui;
    Vision *eye;
    SettingsDialog *serial_sett;
    Serial *serial;
    SettingsDialog::Settings settings;
};

#endif // SOCCER_WINDOW_H