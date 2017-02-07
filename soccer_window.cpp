#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <QDebug>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "soccer_window.h"
#include "ui_soccer_window.h"
#include "cpo.h"
#include "serial.h"
#include "utils.h"

using namespace std;

soccer_window::soccer_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::soccer_window)
{

    ui->setupUi(this);
    area_read = false;
    thread = new QThread;
    serial_sett = new SettingsDialog;
    serial = new Serial;
    eye = new Vision;
    cph = new CPH;
    eye->set_mode(0);

    connect(serial_sett, SIGNAL(serial_settings(SettingsDialog::Settings)), this, SLOT(updateSerialSettings(SettingsDialog::Settings)));
    connect(thread, SIGNAL(started()), cph, SLOT(process()));
    connect(cph, SIGNAL(finished()), thread, SLOT(quit()));
    connect(cph, SIGNAL(finished()), cph, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(eye, SIGNAL(processedImage(QImage)), this, SLOT(updateVisionUI(QImage)));
    connect(eye, SIGNAL(framesPerSecond(double)), this, SLOT(updateFPS(double)));
    connect(eye, SIGNAL(infoPercepted(Perception)), this, SLOT(updatePerceptionInfo(Vision::Perception)));
}

void soccer_window::closeEvent(QCloseEvent *event){
    QWidget::closeEvent(event);

    eye->Stop();
    eye->wait();
    eye->release_cam();
}

void soccer_window::updateVisionUI(QImage img){
    if(!img.isNull()){
        ui->game_view->setAlignment(Qt::AlignCenter);
        ui->game_view->setPixmap(QPixmap::fromImage(img).scaled(ui->game_view->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

void soccer_window::updatePerceptionInfo(Vision::Perception percep_info){
    p2dVector enemy_pos(3), team_pos(3);

    percep = percep_info;

    if(!area_read){
        map_area = percep.map_area;
        atk_area = percep.atk_area;
        def_area = percep.def_area;
        area_read = true;
    }

    if(percep.ball_found){
        ui->ball_detec_col_label->setStyleSheet("QLabel { background-color : green; }");
        ui->ball_detec_label->setText("Ball found");
        ball_pos = percep.ball_pos_cm;
    }else{
        ui->ball_detec_col_label->setStyleSheet("QLabel { background-color : red; }");
        ui->ball_detec_label->setText("Ball not found");
    }

    enemy_pos[0] = percep.enemy_robots[0].get_pos();
    enemy_pos[1] = percep.enemy_robots[1].get_pos();
    enemy_pos[2] = percep.enemy_robots[2].get_pos();

    team_pos[0] = percep.team_robots[0].get_pos();
    team_pos[1] = percep.team_robots[1].get_pos();
    team_pos[2] = percep.team_robots[2].get_pos();

    cph->set_ball_pos(ball_pos);
    cph->set_enemy_pos(enemy_pos);
    cph->set_team_pos(team_pos);

    if(percep.team_robots[1].is_detected()){
        ui->gandalf_detec_col_label->setStyleSheet("QLabel { background-color : green; }");
        ui->gandalf_detec_label->setText("Detected");
    }else{
        ui->gandalf_detec_col_label->setStyleSheet("QLabel { background-color : red; }");
        ui->gandalf_detec_label->setText("Not Detected");
    }
    if(percep.team_robots[0].is_detected()){
        ui->leona_detec_col_label->setStyleSheet("QLabel { background-color : green; }");
        ui->leona_detec_label->setText("Detected");
    }else{
        ui->leona_detec_col_label->setStyleSheet("QLabel { background-color : red; }");
        ui->leona_detec_label->setText("Not Detected");
    }
    if(percep.team_robots[2].is_detected()){
        ui->presto_detec_col_label->setStyleSheet("QLabel { background-color : green; }");
        ui->presto_detec_label->setText("Detected");
    }else{
        ui->presto_detec_col_label->setStyleSheet("QLabel { background-color : red; }");
        ui->presto_detec_label->setText("Not Detected");
    }
}

void soccer_window::updateFPS(double fps){
    ui->fps_lcd->display(fps);
}

void soccer_window::updateSerialSettings(SettingsDialog::Settings settings){
    this->settings = settings;
    serial->set_serial_settings(settings);
}

void soccer_window::on_start_game_clicked()
{
    int cam_id = CV_CAP_FIREWIRE;

    if(eye->isStopped()){
        if(!eye->open_camera(cam_id)){
            QMessageBox msgBox;
            msgBox.setText("The camera could not be opened!");
            msgBox.exec();
        }

        eye->Play();

        ui->start_game->setText("Stop Game");
    }else{
        eye->Stop();
        eye->wait();
        eye->release_cam();
        ui->start_game->setText("Start Game");
    }
}

void soccer_window::on_switch_fields_clicked()
{
    pVector aux;

    aux = atk_area;
    atk_area = def_area;
    def_area = aux;

    eye->switch_teams_areas();
    eye->set_atk_area(atk_area);
    eye->set_def_area(def_area);
}

soccer_window::~soccer_window()
{
    delete eye;
    delete serial;
    delete serial_sett;
    delete ui;
}

void soccer_window::on_read_parameters_clicked()
{
    int ch;
    char cwd[1024];
    vector<Robot> robots = eye->get_robots();
    vector<int> low_color(3);
    vector<int> upper_color(3);
    vector<int> low_team_color(3);
    vector<int> upper_team_color(3);
    pair<vector<int>, vector<int> > ball_range;
    string path, role, ID;
    ifstream file, t1_file, t2_file, ball;

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        fprintf(stdout, "Current working dir: %s\n", cwd);
    else
        perror("getcwd() error");

    ball_range.first.resize(3);
    ball_range.second.resize(3);

    t1_file.open("Config/T1", fstream::in);

    if(!t1_file){
        cout << "Team 1 config could not be opened!" << endl;
    }

    t1_file >> low_team_color[0] >> low_team_color[1] >> low_team_color[2];
    t1_file >> upper_team_color[0] >> upper_team_color[1] >> upper_team_color[2];
    cout << low_team_color[0] << " " << upper_team_color[0] << endl;
    t1_file.close();
    t1_file.clear();

    for(auto itr = robots.begin(); itr != robots.end(); ++itr){
        path = "Config/" + (*itr).get_nick();
        file.open(path.c_str());

        if(!file){
            cout << (*itr).get_nick() << " config could not be opened!" << endl;
        }

        file >> low_color[0] >> low_color[1] >> low_color[2];
        file >> upper_color[0] >> upper_color[1] >> upper_color[2];
        file >> ch;
        file >> role;
        file >> ID;

        (*itr).set_team_low_color(low_team_color);
        (*itr).set_team_upper_color(upper_team_color);
        (*itr).set_low_color(low_color);
        (*itr).set_upper_color(upper_color);
        (*itr).set_ID(ID);
        (*itr).set_channel(ch);
        (*itr).set_role(role);

        file.close();
        file.clear();
    }

    t2_file.open("Config/T2", fstream::in);

    if(!t2_file){
        cout << "Team 2 config could not be opened!" << endl;
    }

    t2_file >> low_team_color[0] >> low_team_color[1] >> low_team_color[2];
    t2_file >> upper_team_color[0] >> upper_team_color[1] >> upper_team_color[2];
    t2_file.close();
    t2_file.clear();

    for(auto itr = robots.begin() + 3; itr != robots.end(); ++itr){
        (*itr).set_team_low_color(low_team_color);
        (*itr).set_team_upper_color(upper_team_color);
    }

    eye->set_robots(robots);
    ui->label->setText(QString::fromStdString(robots[0].get_role()));
    ui->label_2->setText(QString::fromStdString(robots[1].get_role()));
    ui->label_3->setText(QString::fromStdString(robots[2].get_role()));
    ball.open("Config/ball", fstream::in);

    if(!ball){
        cout << "Ball config could not be opened!" << endl;
    }

    ball >> ball_range.first[0] >> ball_range.first[1] >> ball_range.first[2];
    ball >> ball_range.second[0] >> ball_range.second[1] >> ball_range.second[2];
    ball.close();
    ball.clear();

    eye->set_ball(ball_range);
}

void soccer_window::on_CPH_clicked()
{
    //cph->print_grid();
    p2dVector enemy_pos(3);

    enemy_pos[0] = robots[3].get_pos();
    enemy_pos[1] = robots[4].get_pos();
    enemy_pos[2] = robots[5].get_pos();
    if(cph->isStopped()){
        cph->set_enemy_pos(enemy_pos);
        cph->set_ball_pos(ball_pos);
        cph->moveToThread(thread);
        thread->start();
        //if(cph->is_running()) cout << "hey monkey" << endl;
    }else{
        thread->exit();
        //cph->Stop();
        //cph->wait();
    }
}


void soccer_window::on_checkBox_toggled(bool checked)
{
    eye->show_area(checked);
}

void soccer_window::on_checkBox_2_toggled(bool checked)
{
    eye->show_names(checked);
}

void soccer_window::on_checkBox_3_toggled(bool checked)
{
    eye->show_centers(checked);
}
