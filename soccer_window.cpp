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
#include "cph.h"
#include "cpo.h"
#include "fuzzy.h"
#include "utils.h"
#include "mover.h"

using namespace std;

soccer_window::soccer_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::soccer_window)
{
    qRegisterMetaType<Vision::Perception>("Vision::Perception");
    qRegisterMetaType<Selector>("Selector");
    qRegisterMetaType<rVector>("rVector");


    ui->setupUi(this);
    ui->cam_id_spinBox->setValue(0);
    area_read = false;
    eye = new Vision;
    cph = new CPH; //instancia o objeto cph na rotina do sistema
    cpo = new CPO; //instancia o objeto cpo na rotina do sistema
    cph2 = new CPH2; //instancia o objeto cph na rotina do sistema
    cpo2 = new CPO2;
    fuzzy = new Fuzzy; //instancia o objeto fuzzy na rotina do sistema
    mover = new Mover; //instancia o objeto mover na rotina do sistema
    run_cph = false; //flag da thread do cph
    run_cpo = false; //flag da thread do cpo
    run_fuzzy = false; //flag da thread do fuzzy
    run_mover = false;
    team_robots.resize(3);

    eye->set_mode(0);
    load_serial_cfg();
    Robot::config_serial(serial_config);

    connect(eye, SIGNAL(processedImage(QImage)), this, SLOT(updateVisionUI(QImage)));
    connect(eye, SIGNAL(framesPerSecond(double)), this, SLOT(updateFPS(double)));
    connect(eye, SIGNAL(infoPercepted(Vision::Perception)), this, SLOT(updatePerceptionInfo(Vision::Perception)), Qt::QueuedConnection);
    connect(fuzzy, SIGNAL(emitRobots(Selector)), this, SLOT(updateFuzzyRobots(Selector)), Qt::QueuedConnection);
    connect(mover, SIGNAL(emitRobots(Selector)), this, SLOT(updateMoverRobots(Selector)), Qt::QueuedConnection);
    connect(this, SIGNAL(updateVisionInfo(rVector)), eye, SLOT(updateFuzzyRobots(rVector)), Qt::QueuedConnection);
    connect(this, SIGNAL(updateVisionInfo(rVector)), eye, SLOT(updateMoverRobots(rVector)), Qt::QueuedConnection);
}

void soccer_window::load_serial_cfg(){
    fstream in;
    string name;
    int flowControl, parity, stopBits, dataBits;

    in.open("Config/serial.cfg", fstream::in);

    in >> name;
    in >> serial_config.baudRate;
    in >> flowControl;
    in >> parity;
    in >> stopBits;
    in >> dataBits;

    in.close();

    serial_config.name = QString::fromStdString(name);
    serial_config.flowControl = QSerialPort::FlowControl(flowControl);
    serial_config.parity = QSerialPort::Parity(parity);
    serial_config.stopBits = QSerialPort::StopBits(stopBits);
    serial_config.dataBits = QSerialPort::DataBits(dataBits);
}

void soccer_window::closeEvent(QCloseEvent *event){
    QWidget::closeEvent(event);

    eye->Stop();
    eye->wait();
    eye->release_cam();
}

void soccer_window::receiveSerialSettings(SettingsDialog::Settings serial_config){
    Robot::config_serial(serial_config);
}

void soccer_window::updateVisionUI(QImage img){
    if(!img.isNull()){
        ui->game_view->setAlignment(Qt::AlignCenter);
        ui->game_view->setPixmap(QPixmap::fromImage(img).scaled(ui->game_view->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

void soccer_window::updateFuzzyRobots(Selector selec_robot){
    team_robots[0].set_flag_fuzzy(selec_robot.r3.get_flag_fuzzy());
    team_robots[1].set_flag_fuzzy(selec_robot.r1.get_flag_fuzzy());
    team_robots[2].set_flag_fuzzy(selec_robot.r2.get_flag_fuzzy());

    emit updateVisionInfo(team_robots);

   /* cout << team_robots[0].get_nick() << " " << team_robots[0].get_l_vel() << " " << team_robots[0].get_flag_fuzzy() << endl;
    cout << team_robots[1].get_nick() << " " << team_robots[1].get_l_vel() << " " << team_robots[1].get_flag_fuzzy() << endl;
    cout << team_robots[2].get_nick() << " " << team_robots[2].get_l_vel() << " " << team_robots[2].get_flag_fuzzy() << endl;
*/
}

void soccer_window::updateMoverRobots(Selector selec_robot){
    team_robots[0].set_lin_vel(make_pair(selec_robot.r3.get_l_vel(), selec_robot.r3.get_r_vel()));
    team_robots[1].set_lin_vel(make_pair(selec_robot.r1.get_l_vel(), selec_robot.r1.get_r_vel()));
    team_robots[2].set_lin_vel(make_pair(selec_robot.r2.get_l_vel(), selec_robot.r2.get_r_vel()));

    emit updateVisionInfo(team_robots);
}

void soccer_window::updatePerceptionInfo(Vision::Perception percep_info){
    p2dVector enemy_pos(3), team_pos(3);

    percep = percep_info;

    if(!area_read){
        map_area = percep.map_area;
        atk_area = percep.atk_area;
        def_area = percep.def_area;
        area_read = true;
        centroid_atk = (atk_area[2] + atk_area[3] + atk_area[4] + atk_area[5])/4;
        centroid_def = (def_area[2] + def_area[3] + def_area[4] + def_area[5])/4;

        centroid_atk.x = centroid_atk.x * X_CONV_CONST;
        centroid_atk.y = centroid_atk.y * Y_CONV_CONST;

        centroid_def.x = centroid_def.x * X_CONV_CONST;
        centroid_def.y = centroid_def.y * Y_CONV_CONST;

        fuzzy->set_centroid_atk(centroid_atk); //salva a area de atk para o fuzzy
        fuzzy->set_centroid_def(centroid_def); //salva a area de def para o fuzzy

        cpo->set_centroid_atk(centroid_atk);  //salva a area de atk para o cpo
        cpo->set_centroid_def(centroid_def); //salva a area de def para o cpo

        cph->set_centroid_atk(centroid_atk);  //salva a area de atk para o cph
        cph->set_centroid_def(centroid_def); //salva a area de def para o cph
        cph->set_def_area(def_area);

        cpo2->set_centroid_atk(centroid_atk);  //salva a area de atk para o cpo
        cpo2->set_centroid_def(centroid_def); //salva a area de def para o cpo

        cph2->set_centroid_atk(centroid_atk);  //salva a area de atk para o cph
        cph2->set_centroid_def(centroid_def); //salva a area de def para o cph

        mover->set_centroid_atk(centroid_atk);
        mover->set_centroid_def(centroid_def);
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

   /* cout << "Bola :" << "em x: "<< ball_pos.x << " em y: "<< ball_pos.y << endl;
    cout << "Inimigo 1: " << "em x: "<< enemy_pos[0].x << " em y: "<< enemy_pos[0].y << endl;
    cout << "Inimigo 2: " << "em x: "<< enemy_pos[1].x << " em y: "<< enemy_pos[1].y << endl;
    cout << "Inimigo 3: " << "em x: "<< enemy_pos[2].x << " em y: "<< enemy_pos[2].y << endl;*/

    team_pos[0] = percep.team_robots[0].get_pos(); //Leona
    team_pos[1] = percep.team_robots[1].get_pos(); //Gandalf
    team_pos[2] = percep.team_robots[2].get_pos(); //Presto
    team_robots = percep.team_robots;
    //cout << "Presto " << percep.team_robots[2].get_channel() << endl;

    cph->set_ball_pos(ball_pos); //Salva a posicao da bola para o cph
    cph->set_enemy_pos(enemy_pos); //Salva a posicao dos inimigos para o cph
    cph->set_team_pos(team_pos); //Salva a posicao do time para o cph

    cpo->set_ball_pos(ball_pos); //Salva a posicao da bola para o cpo
    cpo->set_enemy_pos(enemy_pos); //Salva a posicao dos inimigos para o cpo
    cpo->set_team_pos(team_pos); //Salva a posicao do time para o cpo
    cph2->set_ball_pos(ball_pos); //Salva a posicao da bola para o cph
    cph2->set_enemy_pos(enemy_pos); //Salva a posicao dos inimigos para o cph
    cph2->set_team_pos(team_pos); //Salva a posicao do time para o cph

    cpo2->set_ball_pos(ball_pos); //Salva a posicao da bola para o cpo
    cpo2->set_enemy_pos(enemy_pos); //Salva a posicao dos inimigos para o cpo
    cpo2->set_team_pos(team_pos); //Salva a posicao do time para o cpo
    //set_to_select(percep.team_robots[1], percep.team_robots[2], percep.team_robots[0]);

    fuzzy->set_to_select(percep.team_robots[1], percep.team_robots[2], percep.team_robots[0]); //Gandalf, Presto e Leona nesta ordem
    fuzzy->set_ball_pos(ball_pos); //Salva a posicao da bola para o fuzzy
    fuzzy->set_enemy_pos(enemy_pos); //Salva a posicao dos inimigos para o fuzzy

    mover->set_to_select(percep.team_robots[1], percep.team_robots[2], percep.team_robots[0]);
    mover->set_to_select_iterador(cph,cpo,cph2,cpo2);
    mover->set_enemy_pos(enemy_pos);
    mover->set_ball_pos(ball_pos);

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
    //cout << "Centroid atk = x: " << centroid_atk.x * X_CONV_CONST << " y: " << centroid_atk.y * Y_CONV_CONST << endl;


    cph->zera_flag_finish();
    cpo->zera_flag_finish();
    cph2->zera_flag_finish();
    cpo2->zera_flag_finish();
    fuzzy->zera_flag_finish();

    //inicia a thread do cph caso ela nao esteja em execucao
    if(run_cph){
        if(cph->is_running()){
            cph->wait();
        }
        //cph->print_grid();
        cph->Play();
     }

    if(run_cph2){
        if(cph2->is_running()){
            cph2->wait();
        }
        //cph2->print_grid();
        cph2->Play();
     }

    //inicia a thread do cpo caso ela nao esteja em execucao
    if(run_cpo){
        if(cpo->is_running()){
            cpo->wait();
        }
        //cpo->print_grid();
        cpo->Play();
     }

    if(run_cpo2){
        if(cpo2->is_running()){
            cpo2->wait();
        }
        //cpo2->print_grid();
        cpo2->Play();
     }

    //inicia a thread do fuzzy caso ela nao esteja em execucao
    if(run_fuzzy){
        if(fuzzy->is_running()){
            fuzzy->wait();
        }
        fuzzy->Play();
     }

    if(run_mover){
        if(mover->is_running()){
            mover->wait();
        }
        mover->Play();
     }

   //cout<<percep.team_robots[1].get_channel()<<endl;
   float vel = percep.team_robots[1].get_l_vel();
   float ver = percep.team_robots[1].get_r_vel();
   //cout<<vel<<endl;
   //cout<<ver<<endl;

   /*if(euclidean_dist(cph2->get_meta_aux(), team_robots[1].get_pos()) < 8){
       Robot::send_velocities(team_robots[1].get_channel(),make_pair(0, 0));
   }else{
       Robot::send_velocities(team_robots[1].get_channel(),make_pair(team_robots[1].get_r_vel(), team_robots[1].get_l_vel()));
   }*/

   Robot::send_velocities(team_robots[1].get_channel(),make_pair(team_robots[1].get_r_vel(), team_robots[1].get_l_vel()));
   //cout << "channel " << team_robots[1].get_channel() << endl;

}

void soccer_window::updateFPS(double fps){
    ui->fps_lcd->display(fps);
}

void soccer_window::updateSerialSettings(SettingsDialog::Settings settings){
    //this->settings = settings;
    //serial->set_serial_settings(settings);
}

void soccer_window::on_start_game_clicked()
{
    int cam_id = ui->cam_id_spinBox->value();

    if(eye->isStopped()){
        if(!eye->open_camera(cam_id)){
            QMessageBox msgBox;
            msgBox.setText("The camera could not be opened!");
            msgBox.exec();
            return;
        }

        eye->Play();
        Robot::open_serial();
        ui->start_game->setText("Stop Game");
    }else{
        eye->Stop();
        eye->wait();
        eye->release_cam();
        Robot::send_velocities(team_robots[1].get_channel(), make_pair(0, 0));
        Robot::close_serial();
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

    centroid_atk = (atk_area[2] + atk_area[3] + atk_area[4] + atk_area[5])/4;
    centroid_def = (def_area[2] + def_area[3] + def_area[4] + def_area[5])/4;

    centroid_atk.x = centroid_atk.x * X_CONV_CONST;
    centroid_atk.y = centroid_atk.y * Y_CONV_CONST;

    centroid_def.x = centroid_def.x * X_CONV_CONST;
    centroid_def.y = centroid_def.y * Y_CONV_CONST;

    fuzzy->set_centroid_atk(centroid_atk); //salva a area de atk para o fuzzy
    fuzzy->set_centroid_def(centroid_def); //salva a area de def para o fuzzy

    cpo->set_centroid_atk(centroid_atk);  //salva a area de atk para o cpo
    cpo->set_centroid_def(centroid_def); //salva a area de def para o cpo

    cph->set_centroid_atk(centroid_atk);  //salva a area de atk para o cph
    cph->set_centroid_def(centroid_def); //salva a area de def para o cph

    cpo2->set_centroid_atk(centroid_atk);  //salva a area de atk para o cpo
    cpo2->set_centroid_def(centroid_def); //salva a area de def para o cpo

    cph2->set_centroid_atk(centroid_atk);  //salva a area de atk para o cph
    cph2->set_centroid_def(centroid_def); //salva a area de def para o cph

    mover->set_centroid_atk(centroid_atk);
    mover->set_centroid_def(centroid_def);
}

soccer_window::~soccer_window()
{
    delete eye;
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
        cerr << "Team 1 config could not be opened!" << endl;
    }

    t1_file >> low_team_color[0] >> low_team_color[1] >> low_team_color[2];
    t1_file >> upper_team_color[0] >> upper_team_color[1] >> upper_team_color[2];

    t1_file.close();
    t1_file.clear();

    for(auto itr = robots.begin(); itr != robots.end(); ++itr){
        path = "Config/" + (*itr).get_nick();
        file.open(path.c_str());

        if(!file){
            cerr << (*itr).get_nick() << " config could not be opened!" << endl;
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
        cerr << "Team 2 config could not be opened!" << endl;
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
    ui->gandalf_role_label->setText(QString::fromStdString(robots[1].get_role()));
    ui->leona_role_label->setText(QString::fromStdString(robots[0].get_role()));
    ui->presto_role_label->setText(QString::fromStdString(robots[2].get_role()));
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
    if(!run_cph){
        run_cph = true;
    }else{
        run_cph = false;
    }
    if(!run_cpo){
        run_cpo = true;
    }else{
        run_cpo = false;
    }
    if(!run_cph2){
        run_cph2 = true;
    }else{
        run_cph2 = false;
    }
    if(!run_cpo2){
        run_cpo2 = true;
    }else{
        run_cpo2 = false;
    }
 /*   if(!run_mover){
        run_mover = true;
    }else run_mover = false;*/
    if(!run_fuzzy){
        run_fuzzy = true;
    }else{
        run_fuzzy = false;
    }
}

void soccer_window::on_show_field_areas_checkbox_toggled(bool checked)
{
    eye->show_area(checked);
}

void soccer_window::on_show_rnames_checkBox_toggled(bool checked)
{
    eye->show_names(checked);
}

void soccer_window::on_show_rcentroids_checkbox_toggled(bool checked)
{
    eye->show_centers(checked);
}

void soccer_window::on_show_visionlogs_checkbox_toggled(bool checked)
{
    eye->show_errors(checked);
}

void soccer_window::on_pushButton_clicked()
{
    /*static int contador = 0;
    if(contador == 0)
    {
        Robot::open_serial();
        Robot::send_velocities(3, make_pair(-0.1, -0.1));
        //Robot::send_velocities(2, make_pair(0.5, 0.5));
        contador++;
    }
    else if(contador == 1)
    {
        Robot::send_velocities(3, make_pair(0, 0));
        //Robot::send_velocities(2, make_pair(0, 0));
        contador++;
    }
    else if(contador == 2)
    {
        Robot::send_velocities(3, make_pair(-0.5, -0.5));
        //Robot::send_velocities(2, make_pair(0.2, 0.2));
        contador=1;
    }*/

    /*if(!run_mover){
        run_mover = true;
    }else{
        run_mover = false;
    }*/

    if (cph->get_flag_finish() && cpo->get_flag_finish() && cph2->get_flag_finish() && cpo2->get_flag_finish() &&  fuzzy->get_flag_finish() && !run_mover){
        run_mover = true;
    }else{
        run_mover = false;
    }
   /* if (cph2->get_flag_finish() && cpo2->get_flag_finish() && fuzzy->get_flag_finish() && !run_mover){
        run_mover = true;
    }else{
        run_mover = false;
    }*/
}
