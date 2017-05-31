#include <iostream>
#include <iomanip>
#include <cmath>
#include "mover.h"
#include "robot.h"
#include "utils.h"

using namespace std;
double limiar_theta = 90;
double v_max = 0.6;
double w_max = 7;
double v_max_gol = 0.4;
double w_max_gol = 5;
double k = (w_max/v_max);
double l = 0.028; // caso mudar de robo trocar esse valor (robo antigo 0.0275)

double v,w,theta,alpha;
pair<float, float> vel;

Serial Mover::serial;

Mover::Mover()
{
    stop = true;
    mover_initialized = false;
    enemy_pos_grid = pVector(3);
    team_pos_grid = pVector(3);
    team_chang = false;
    vels.assign(3, make_pair(0, 0));

    i = 0;
    stop = true;
    grid_initialized = false;
    enemy_pos_grid = pVector(3);
    team_pos_grid = pVector(3);
    pGrid = dMatrix(28, vector<double>(36, 0.0));
    tGrid = dMatrix(28, vector<double>(36, 0.0));
}

Mover::~Mover(){

}

bool Mover::isStopped() const
{
    return this->stop;
}

void Mover::Play(){
    if(isStopped())
        stop = false;
   start();
}

void Mover::Stop(){
    stop = true;
}

bool Mover::is_running(){
    return isRunning();
}

void Mover::msleep(int ms){
    struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
}

void Mover::run(){
    if(!mover_initialized){
        //init_mover();
        mover_initialized = true;
    }
    //Pro terceiro robô - Leona
    /*int r3_flag = selec_robot.r3.get_flag_fuzzy();
    switch (r3_flag){
        case 0:
            defender(&selec_robot.r3, 2, &vels[2]);
            break;
        case 1:
            defensive_midfielder(&selec_robot.r3, 2, &vels[2]);
            break;
        case 2:
            ofensive_midfielder(&selec_robot.r3, 2, &vels[2]);
            break;
        case 3:
            striker(&selec_robot.r3, 2, &vels[2]);
            break;
        case 4:
            goalkeeper(&selec_robot.r3, 2, &vels[2]);
            break;
    }
    selec_robot.r3.set_lin_vel(vels[2]);*/

    //Pro primeiro robô - Gandalf
    int r1_flag = selec_robot.r1.get_flag_fuzzy();
    switch (r1_flag){
        case 0:
            defender(&selec_robot.r1, 0, &vels[0]);
            break;
        case 1:
            defensive_midfielder(&selec_robot.r1, 0, &vels[0]);
            break;
        case 2:
            ofensive_midfielder(&selec_robot.r1, 0, &vels[0]);
            break;
        case 3:
            striker(&selec_robot.r1, 0, &vels[0]);
            break;
        case 4:
            goalkeeper(&selec_robot.r1, 0, &vels[0]);
            break;
    }
    selec_robot.r1.set_lin_vel(vels[0]);
    /*
    //Pro segundo robô - Presto
    int r2_flag = selec_robot.r2.get_flag_fuzzy();
    switch (r2_flag){
        case 0:
            defender(&selec_robot.r2, 1, &vels[1]);
            break;
        case 1:
            defensive_midfielder(&selec_robot.r2, 1, &vels[1]);
            break;
        case 2:
            ofensive_midfielder(&selec_robot.r2, 1, &vels[1]);
            break;
        case 3:
            striker(&selec_robot.r2, 1, &vels[1]);
            break;
        case 4:
            goalkeeper(&selec_robot.r2, 1, &vels[1]);
        break;
    }
    selec_robot.r2.set_lin_vel(vels[1]);*/


    emit emitRobots(selec_robot);
}

void Mover::set_to_select(Robot r1, Robot r2, Robot r3){
    selec_robot.r1 = r1;
    selec_robot.r2 = r2;
    selec_robot.r3 = r3;
}

void Mover::set_to_select_iterador(CPH *cph, CPO *cpo, CPH2 *cph2, CPO2 *cpo2, CPO3 *cpo3){
    /*selec_iterador.Gandalf = Gandalf;
    selec_iterador.Presto = Presto;
    selec_iterador.Leona = Leona;*/
    /*selec_iterador.cph = cph;
    selec_iterador.cpo = cpo;
    selec_iterador.cph2 = cph2;
    selec_iterador.cpo2 = cpo2;
    selec_iterador.cpo3 = cpo3;*/
}

void Mover::set_ball_vel(pair<double, double> ball_vel){
    this->ball_vel = ball_vel;
}

double Mover::set_ang(double robot_angle, double angle, double w){
    int acceptance;
    double ang, err;

    acceptance = 10;
    err = angle - robot_angle;

    if(fabs(err) > acceptance){
        if(err < 0){
            w = -w;
        }
    }else w = 0;

    return w;
}

void Mover::team_changed(){
    if(!team_chang){
        team_chang = true;
    }else{
        team_chang = false;
    }
}

void Mover::calcula_velocidades(Robot *r, CPH *cph,CPO *cpo, CPH2 *cph2, CPO2 *cpo2, CPO3 *cpo3, pair<float, float> *vels){

    double v,w,theta,alpha;
    pair<float, float> vel;

    Point2d robot_pos = r->get_pos();
    Point robot_grid = convert_C_to_G(robot_pos);

    Point2d ball_v;

    ball_v.x = ball_vel.first / 100;
    ball_v.y = -ball_vel.second / 100;
    //cout << " Posicao do robo " << endl;
    //cout << " x: " << robot_pos.x << " y: " << robot_pos.y << endl;
   // cout << r->get_flag_fuzzy() << endl;


    if(r->get_flag_fuzzy() == 4){
         /*goleiro(*r, vels);
         v = vels->first;
         w = vels->second;
         v = 0;

         if(ball_pos.y < r->get_pos().y){
             w = -7;
         }else{
             w = 7;
         }
         theta = cpo3->get_direction(robot_grid);
         alpha = theta - r->get_angle();
         alpha = ajusta_angulo(alpha);

         //cout << "angulo do robo: "<<r->get_angle()<<endl;

         if (fabs(alpha) <= limiar_theta){
             w = k*v_max*alpha/180;
             v = -v_max*fabs(alpha)/limiar_theta + v_max;
         }
         else{
             alpha = ajusta_angulo(alpha+180);
             w = k*v_max*alpha/180;
             v = v_max*fabs(alpha)/limiar_theta - v_max;
         }
         Point2d meta_aux;

         //cout << "velocidade linear: " << v << endl;
         //cout << "velocidade angular: " << w << endl;



         if((euclidean_dist(robot_pos, cpo3->get_meta_aux()) < 4) && (fabs(r->get_angle())> 80) && (fabs(r->get_angle()< 100))){
             v = 0;
             w = 0;
         }
         else if (euclidean_dist(robot_pos, cpo3->get_meta_aux()) < 4){
             v = 0;
             alpha = theta - 90;
             w = k*v_max*alpha/180;
         }
         vels->first = v - w*l;
         vels->second = v + w*l;*/

        theta = cpo3->get_direction(robot_grid);
        alpha = theta - r->get_angle();
        alpha = ajusta_angulo(alpha);

        if (ball_pos.x < centroid_atk.x){
            if(ball_pos.x > 90 || ball_pos.y < 45 || ball_pos.y > 95){
                if (fabs(alpha) <= limiar_theta){
                    w = k*v_max_gol*alpha/180;
                    v = -v_max_gol*fabs(alpha)/limiar_theta + v_max_gol;
                }
                else{
                    alpha = ajusta_angulo(alpha+180);
                    w = k*v_max_gol*alpha/180;
                    v = v_max_gol*fabs(alpha)/limiar_theta - v_max_gol;
                }

                //Return2Goal
                vels->first = v - w*l;
                vels->second = v + w*l;

                //AdjustRobo
                if((euclidean_dist(robot_pos, cpo3->get_meta_aux()) < 5) && (fabs(r->get_angle()) > 85) && (fabs(r->get_angle()) < 95)){
                    vels->first = 0;
                    vels->second = 0;
                }
                else if (euclidean_dist(robot_pos, cpo3->get_meta_aux()) < 5){
                    goalkeeper_orientation(r,vels);
                }
            }
            else{
                //AdjustRobo
                if(fabs(r->get_angle()) < 80 || fabs(r->get_angle()) > 100)
                    goalkeeper_orientation(r,vels);
                else{
                    //FollowBall
                    if (r->get_angle() > 0 && ball_pos.y < robot_pos.y ){
                        v = fabs(ball_v.y) - 0.01*(ball_pos.y-robot_pos.y);
                    }
                    else if (r->get_angle() > 0 && ball_pos.y > robot_pos.y){
                        v = -fabs(ball_v.y) - 0.01*(ball_pos.y-robot_pos.y);
                    }
                    else if (r->get_angle() < 0 && ball_pos.y < robot_pos.y ){
                         v = -fabs(ball_v.y) + 0.01*(ball_pos.y-robot_pos.y);
                    }
                    else if (r->get_angle() < 0 && ball_pos.y > robot_pos.y ){
                         v = fabs(ball_v.y) + 0.01*(ball_pos.y-robot_pos.y);
                    }
                    vels->first = v;
                    vels->second = v;
                }
            }
            // previsao de bola - perigo de gol

           double tempo;
           double aux_position_y;
           if (r->get_angle() > 0 && ball_pos.y < robot_pos.y ){
               if (ball_v.x < -0.4){
                   tempo = (centroid_def.x+5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y > robot_pos.y){
                       v = -(aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
           else if (r->get_angle() > 0 && ball_pos.y > robot_pos.y){
               if (ball_v.x < -0.4){
                   tempo = (centroid_def.x+5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y < robot_pos.y){
                       v = -(aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
           else if (r->get_angle() < 0 && ball_pos.y < robot_pos.y ){
               if (ball_v.x < -0.4){
                   tempo = (centroid_def.x+5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y > robot_pos.y){
                       v = (aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
           else if (r->get_angle() < 0 && ball_pos.y > robot_pos.y ){
               if (ball_v.x < -0.4){
                   tempo = (centroid_def.x+5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y < robot_pos.y){
                       v = (aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
        }
        else {
            if(ball_pos.x < 90 || ball_pos.y < 45 || ball_pos.y > 95){
                if (fabs(alpha) <= limiar_theta){
                    w = k*v_max_gol*alpha/180;
                    v = -v_max_gol*fabs(alpha)/limiar_theta + v_max_gol;
                }
                else{
                    alpha = ajusta_angulo(alpha+180);
                    w = k*v_max_gol*alpha/180;
                    v = v_max_gol*fabs(alpha)/limiar_theta - v_max_gol;
                }

                //Return2Goal
                vels->first = v - w*l;
                vels->second = v + w*l;

                //AdjustRobo
                if((euclidean_dist(robot_pos, cpo3->get_meta_aux()) < 5) && (fabs(r->get_angle()) > 85) && (fabs(r->get_angle()) < 95)){
                    vels->first = 0;
                    vels->second = 0;
                }
                else if (euclidean_dist(robot_pos, cpo3->get_meta_aux()) < 5){
                    goalkeeper_orientation(r,vels);
                }
            }
            else{
                //AdjustRobo
                if(fabs(r->get_angle()) < 80 || fabs(r->get_angle()) > 100)
                    goalkeeper_orientation(r,vels);
                else{
                    //FollowBall
                    if (r->get_angle() > 0 && ball_pos.y < robot_pos.y ){
                        v = fabs(ball_v.y) - 0.01*(ball_pos.y-robot_pos.y);
                    }
                    else if (r->get_angle() > 0 && ball_pos.y > robot_pos.y){
                        v = -fabs(ball_v.y) - 0.01*(ball_pos.y-robot_pos.y);
                    }
                    else if (r->get_angle() < 0 && ball_pos.y < robot_pos.y ){
                         v = -fabs(ball_v.y) + 0.01*(ball_pos.y-robot_pos.y);
                    }
                    else if (r->get_angle() < 0 && ball_pos.y > robot_pos.y ){
                         v = fabs(ball_v.y) + 0.01*(ball_pos.y-robot_pos.y);
                    }
                    vels->first = v;
                    vels->second = v;
                }
            }
            // previsao de bola - perigo de gol

           double tempo;
           double aux_position_y;
           if (r->get_angle() > 0 && ball_pos.y < robot_pos.y ){
               if (ball_v.x > 0.4){
                   tempo = (centroid_def.x-5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y > robot_pos.y){
                       v = -(aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
           else if (r->get_angle() > 0 && ball_pos.y > robot_pos.y){
               if (ball_v.x > 0.4){
                   tempo = (centroid_def.x-5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y < robot_pos.y){
                       v = -(aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
           else if (r->get_angle() < 0 && ball_pos.y < robot_pos.y ){
               if (ball_v.x > 0.4){
                   tempo = (centroid_def.x-5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y > robot_pos.y){
                       v = (aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }
           else if (r->get_angle() < 0 && ball_pos.y > robot_pos.y ){
               if (ball_v.x > 0.4){
                   tempo = (centroid_def.x-5-ball_pos.x)/ball_v.x;
                   aux_position_y = ball_pos.y - tempo*ball_v.y;
                   //if (aux_position_y < robot_pos.y){
                       v = (aux_position_y-robot_pos.y)/tempo;
                       vels->first = v;
                       vels->second = v;
                   //}
               }
           }

        }


     }
    else{
       if (r->get_flag_fuzzy() == 0){
           theta = cph->get_direction(robot_grid);
       }
       else if (r->get_flag_fuzzy() == 1){
           theta = cph2->get_direction(robot_grid);
       }
       else if (r->get_flag_fuzzy() == 2){
           theta = cpo2->get_direction(robot_grid);
       }
       else if (r->get_flag_fuzzy() == 3){
           theta = cpo->get_direction(robot_grid);
       }
    }
    //cout << "decisao: " << r->get_flag_fuzzy();

        if (centroid_atk.x > ball_pos.x){
            if(r->get_flag_fuzzy() != 4){
                if ((ball_pos.y > centroid_atk.y+55) && (euclidean_dist(ball_pos,robot_pos) < 5)){
                    //cout << "1" << endl;
                    vels->first = -0.6;
                    vels->second = 0.6;
                }
                else if ((ball_pos.y < centroid_atk.y-55) && (euclidean_dist(ball_pos,robot_pos) < 5)){
                    //cout << "2" << endl;
                    vels->first = 0.6;
                    vels->second = -0.6;
                }
                else if (euclidean_dist(robot_pos,cph2->get_meta_aux()) < 5 && r->get_flag_fuzzy() == 1){
                    vels->first = 0;
                    vels->second = 0;
                }
                else{
                    alpha = theta - r->get_angle();
                    alpha = ajusta_angulo(alpha);

                    //cout << "angulo do robo: "<<r->get_angle()<<endl;

                    if (fabs(alpha) <= limiar_theta){
                        w = k*v_max*alpha/180;
                        v = -v_max*fabs(alpha)/limiar_theta + v_max;
                    }
                    else{
                        alpha = ajusta_angulo(alpha+180);
                        w = k*v_max*alpha/180;
                        v = v_max*fabs(alpha)/limiar_theta - v_max;
                    }


                    //cout << "velocidade linear: " << v << endl;
                    //cout << "velocidade angular: " << w << endl;

                    vels->first = v - w*l;
                    vels->second = v + w*l;
                    //r->set_lin_vel(make_pair(vl,vr));
                }
            }
        }else{
            if(r->get_flag_fuzzy() != 4){
                if ((ball_pos.y > centroid_atk.y+55) && (euclidean_dist(ball_pos,robot_pos) < 5)){
                    vels->first = 0.6;
                    vels->second = -0.6;
                }
                else if ((ball_pos.y < centroid_atk.y-55) && (euclidean_dist(ball_pos,robot_pos) < 5)){
                    vels->first = -0.6;
                    vels->second = 0.6;
                }
                else if (euclidean_dist(robot_pos,cph2->get_meta_aux()) < 5 && r->get_flag_fuzzy() == 1){
                    vels->first = 0;
                    vels->second = 0;
                }
                else{
                    alpha = theta - r->get_angle();
                    alpha = ajusta_angulo(alpha);

                    //cout << "angulo do robo: "<<r->get_angle()<<endl;

                    if (fabs(alpha) <= limiar_theta){
                        w = k*v_max*alpha/180;
                        v = -v_max*fabs(alpha)/limiar_theta + v_max;
                    }
                    else{
                        alpha = ajusta_angulo(alpha+180);
                        w = k*v_max*alpha/180;
                        v = v_max*fabs(alpha)/limiar_theta - v_max;
                    }

                    //cout << "velocidade linear: " << v << endl;
                    //cout << "velocidade angular: " << w << endl;

                    vels->first = v-w*l;
                    vels->second = v+w*l;
                    //cout << vl << " " << vr << endl;
                    //r->set_lin_vel(make_pair(vl,vr));
                }
            }
        }
}

void Mover::goalkeeper_orientation(Robot *r, pair<float, float> *vels){
    double alpha,w;
    alpha = 90 - r->get_angle();
    alpha = ajusta_angulo(alpha);
    if (fabs(alpha) <= limiar_theta){
        w = k*v_max*alpha/180,2;
    }
    else{
        alpha = ajusta_angulo(alpha+180);
        w = k*v_max*alpha/180,2;
    }
    vels->first = -w*l;
    vels->second = w*l;
}

Point Mover::convert_C_to_G(Point2d coord){
    Point i;

    coord.x = int(coord.x) + 5;
    coord.y = int(coord.y) + 5;

    if(coord.x / 5 != 35){
        i.x = coord.x / 5;
    }else{
        i.x = coord.x / 5 - 1;
    }

    if(coord.y / 5 != 27){
        i.y = coord.y / 5;
    }else{
        i.y = coord.y / 5 - 1;
    }
    return i;
}

void Mover::set_enemy_pos(p2dVector enemy_pos){
    this->enemy_pos = enemy_pos;
}

void Mover::set_ball_pos(Point2d ball_pos){
    this->ball_pos = ball_pos;
}

void Mover::set_centroid_atk(Point2d centroid_atk){
    this->centroid_atk = centroid_atk;
}

void Mover::set_centroid_def(Point2d centroid_def){
    this->centroid_def = centroid_def;
}

void Mover::set_team_pos(p2dVector team_pos){
    this->team_pos = team_pos;
}

void Mover::set_def_area(pVector def_area){
    this->def_area = def_area;
}

double Mover::ajusta_angulo(double angle){
    if (angle < -180)
        angle = angle + 360;
    else if (angle > 180)
        angle = angle - 360;
    else
        angle = angle;
    return angle;
}

void Mover::goalkeeper(Robot *robo, int num_Robo, pair<float, float> *vels){
    //fazer depois
}

void Mover::defender(Robot *robo, int num_Robo, pair<float, float> *vels){
    // Gera o grid
    //if(!grid_initialized){
        init_grid();
    //}

    for(i = 0; i < 3; ++i){
        if(enemy_pos[i].x > 0 && enemy_pos[i].y > 0){
            enemy_pos_grid[i] = convert_C_to_G(enemy_pos[i]);
            //cout<<"Inimigo "<<enemy_pos_grid[i].x<<" "<<enemy_pos_grid[i].y<<endl;
            if(enemy_pos_grid[i].x>0 && enemy_pos_grid[i].y>0)
                set_potential(enemy_pos_grid[i].y, enemy_pos_grid[i].x, 1);
        }else{
            //tratar posição dos inimigos aqui
        }

        if(team_pos[i].x > 0 && team_pos[i].y > 0){
            team_pos_grid[i] = convert_C_to_G(team_pos[i]);
            //cout<<"Amigo "<<team_pos_grid[i].x<<" "<<team_pos_grid[i].y<<endl;
        }else{
            //tratar posição dos miguxos aqui
        }
    }

    double def_area_x = def_area[0].x*X_CONV_CONST;
    double def_area_y1 = def_area[1].y*Y_CONV_CONST;
    double def_area_y2 = def_area[6].y*Y_CONV_CONST;
    Point meta;
    Point2d meta2d;
    meta2d.y = centroid_def.y;



    if (centroid_atk.x > ball_pos.x){
        if(ball_pos.x < def_area_x && ball_pos.y < def_area_y1 && ball_pos.y > def_area_y2){
            //cout << "Reconheceu a area de defesa" << endl;


            if(ball_pos.x > 0 && ball_pos.y > 0){
                meta2d.x = centroid_def.x + 35;
                meta = convert_C_to_G(meta2d);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if (meta.x > 0 && meta.y > 0)
                    set_potential(meta.y, meta.x, 0);
            }else{
                //tratar a meta aqui
            }

        }else{
            if(ball_pos.x > 0 && ball_pos.y > 0){
                ball_pos_grid = convert_C_to_G(ball_pos);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if (ball_pos_grid.x > 0 && ball_pos_grid.y > 0)
                    set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);
            }else{
                //tratar a bola aqui
            }

            if(ball_pos.x > 0 && ball_pos.y > 0){
                ball_pos_grid = convert_C_to_G(ball_pos);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if(ball_pos_grid.x+1 < 28 && ball_pos_grid.y+1 < 28){
                    set_potential(ball_pos_grid.y, ball_pos_grid.x+1, 1);
                    set_potential(ball_pos_grid.y+1, ball_pos_grid.x+1, 1);
                    set_potential(ball_pos_grid.y-1, ball_pos_grid.x+1, 1);
                }
            }else{
                //tratar a barreira aqui
            }
        }
    }else{
        if(ball_pos.x > def_area_x && ball_pos.y < def_area_y1 && ball_pos.y > def_area_y2){
            if(ball_pos.x > 0 && ball_pos.y > 0){
                meta2d.x = centroid_def.x - 35;
                meta = convert_C_to_G(meta2d);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if (meta.x > 0 && meta.y > 0)
                    set_potential(meta.y, meta.x, 0);
            }else{
                //tratar a meta aqui
            }
        }else{
            if(ball_pos.x > 0 && ball_pos.y > 0){
                ball_pos_grid = convert_C_to_G(ball_pos);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if (ball_pos_grid.x > 0 && ball_pos_grid.y > 0)
                    set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);
            }else{
                //tratar a bola aqui
            }

            if(ball_pos.x > 0 && ball_pos.y > 0){
                ball_pos_grid = convert_C_to_G(ball_pos);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if(ball_pos_grid.y+1 < 28){
                    set_potential(ball_pos_grid.y, ball_pos_grid.x-1, 1);
                    set_potential(ball_pos_grid.y+1, ball_pos_grid.x-1, 1);
                    set_potential(ball_pos_grid.y-1, ball_pos_grid.x-1, 1);
                }
            }else{
                //tratar a barreira aqui
            }
        }
    }
    while(iterator_cph()>1E-6);
    set_direction();

    // Calcula velocidades
    Point robot_grid = convert_C_to_G(robo->get_pos());

    theta = get_direction(robot_grid);
    alpha = theta - robo->get_angle();

    if (fabs(alpha) <= limiar_theta){
        w = k*v_max*alpha/180;
        v = -v_max*fabs(alpha)/limiar_theta + v_max;
    }
    else{
        alpha = ajusta_angulo(alpha+180);
        w = k*v_max*alpha/180;
        v = v_max*fabs(alpha)/limiar_theta - v_max;
    }

    vels->first = v - w*l;
    vels->second = v + w*l;
}

void Mover::defensive_midfielder(Robot *robo, int num_Robo, pair<float, float> *vels){
    //if(!grid_initialized){
        init_grid();
    //}
    Point2d meta_aux;

    for(i = 0; i < 3; ++i){
        if(enemy_pos[i].x > 0 && enemy_pos[i].y > 0){
            enemy_pos_grid[i] = convert_C_to_G(enemy_pos[i]);
            //cout<<"Inimigo "<<enemy_pos_grid[i].x<<" "<<enemy_pos_grid[i].y<<endl;
            set_potential(enemy_pos_grid[i].y, enemy_pos_grid[i].x, 1);
        }else{
            //tratar posição dos inimigos aqui
        }

        if(team_pos[i].x > 0 && team_pos[i].y > 0){
            team_pos_grid[i] = convert_C_to_G(team_pos[i]);
            //cout<<"Amigo "<<team_pos_grid[i].x<<" "<<team_pos_grid[i].y<<endl;
        }else{
            //tratar posição dos miguxos aqui
        }
    }

    //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
    Point2d meta;

    if(ball_pos.x > 0 && ball_pos.y > 0){
        if ((ball_pos.y > centroid_def.y+55 || ball_pos.y < centroid_def.y-55) && fabs(ball_pos.x - centroid_def.x) < 70){
            //cout << 1 << endl;
            ball_pos_grid = convert_C_to_G(ball_pos);
            if (ball_pos_grid.x > 0 && ball_pos_grid.y > 0){
                //cout << 2 << endl;
                //cout<<"Meta: "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);
                meta_aux = ball_pos;
                /*if(ball_pos.x < centroid_atk.x){
                    set_potential(ball_pos_grid.y, ball_pos_grid.x+1, 1);
                    set_potential(ball_pos_grid.y+1, ball_pos_grid.x+1, 1);
                    set_potential(ball_pos_grid.y-1, ball_pos_grid.x+1, 1);
                }else{
                    set_potential(ball_pos_grid.y, ball_pos_grid.x-1, 1);
                    set_potential(ball_pos_grid.y+1, ball_pos_grid.x-1, 1);
                    set_potential(ball_pos_grid.y-1, ball_pos_grid.x-1, 1);
                }*/
            }
        }else{
            Point2d vec_ball_def = centroid_def - ball_pos;
            double aux = (0.45/150)*euclidean_dist(centroid_def,ball_pos);

            meta = ball_pos + vec_ball_def*aux;
            //cout << "Meta: " << endl;
            //cout << " x: " << meta.x << " y: " << meta.y << endl;
            if(meta.x > 0 && meta.y > 0){
                 Point meta_pos_grid = convert_C_to_G(meta);
                 //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                 if (meta_pos_grid.x > 0 && meta_pos_grid.y > 0)
                     set_potential(meta_pos_grid.y, meta_pos_grid.x, 0);
            }else{
                //tratar a meta aqui
            }
            meta_aux = meta;
        }
    }else{
        //tratar a bola aqui
    }

    /*if(ball_pos.x > 0 && ball_pos.y > 0){
        ball_pos_grid = convert_C_to_G(ball_pos);
        //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
        set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);
    }else{
        //tratar a bola aqui
    }*/
    while(iterator_cph()>1E-6);
    set_direction();

    // Calcula velocidades
    Point robot_grid = convert_C_to_G(robo->get_pos());

    theta = get_direction(robot_grid);
    alpha = theta - robo->get_angle();

    if (fabs(alpha) <= limiar_theta){
        w = k*v_max*alpha/180;
        v = -v_max*fabs(alpha)/limiar_theta + v_max;
    }
    else{
        alpha = ajusta_angulo(alpha+180);
        w = k*v_max*alpha/180;
        v = v_max*fabs(alpha)/limiar_theta - v_max;
    }

    vels->first = v - w*l;
    vels->second = v + w*l;
}

void Mover::ofensive_midfielder(Robot *robo, int num_Robo, pair<float, float> *vels){
    Point2d eixo_x(1.0,0.0);
    Point2d meta2d;
    Point meta;
    //if(!grid_initialized){
        init_grid();
    //}

    for(i = 0; i < 3; ++i){
        if(enemy_pos[i].x > 0 && enemy_pos[i].y > 0){
            enemy_pos_grid[i] = convert_C_to_G(enemy_pos[i]);
            //cout<<"Inimigo "<<enemy_pos_grid[i].x<<" "<<enemy_pos_grid[i].y<<endl;
            if(enemy_pos_grid[i].x>0 && enemy_pos_grid[i].y>0)
                set_potential(enemy_pos_grid[i].y, enemy_pos_grid[i].x, 1);
        }else{
            //tratar posição dos inimigos aqui
        }

        if(team_pos[i].x > 0 && team_pos[i].y > 0){
            team_pos_grid[i] = convert_C_to_G(team_pos[i]);
            //cout<<"Amigo "<<team_pos_grid[i].x<<" "<<team_pos_grid[i].y<<endl;
        }else{
            //tratar posição dos miguxos aquieuclidean_dist(ball_pos,enemy_prox
        }
    }

    if(ball_pos.x > 0 && ball_pos.y > 0){
        if (fabs(ball_pos.x - centroid_def.x) < 40){
            set_epsilon(0);
            set_orientation(0);
            if (ball_pos.x < centroid_atk.x){
                meta2d.x = centroid_def.x + 75;
                meta2d.y  = centroid_def.y;
                meta = convert_C_to_G(meta2d);
                if(meta.x > 0 && meta.y > 0){
                         set_potential(meta.y, meta.x, 0);
                }else{
                    //tratar a meta aqui
                }
            }
            else{
                meta2d.x = centroid_def.x - 75;
                meta2d.y  = centroid_def.y;
                meta = convert_C_to_G(meta2d);
                if(meta.x > 0 && meta.y > 0){
                         set_potential(meta.y, meta.x, 0);
                }else{
                    //tratar a meta aqui
                }
            }
        }
        else{
            //Utiliza o robo amigo mais próximo para definição do epsilon
            Point2d team_prox;
            if ((euclidean_dist(ball_pos,team_pos[0]) <= euclidean_dist(ball_pos,team_pos[1])) && (euclidean_dist(ball_pos,team_pos[0]) <= euclidean_dist(ball_pos,team_pos[2])))
                team_prox = team_pos[0];
            else if (euclidean_dist(ball_pos,team_pos[1]) <= euclidean_dist(ball_pos,team_pos[2]))
                team_prox = team_pos[1];
            else
                team_prox = team_pos[2];

            set_epsilon(0.3 + euclidean_dist(team_prox,ball_pos)/200);
           // cout << " epsilon: " << e << endl;

            ball_pos_grid = convert_C_to_G(ball_pos);
             //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
            if (ball_pos_grid.x > 0 && ball_pos_grid.y > 0)
                set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);

            // Calculo do angulo de orientacao usar no ataque leve para dribles
            //Corrige Posicionamento
            ball_pos.y = -ball_pos.y;
            centroid_atk.y = -centroid_atk.y;

            //Calcula angulo entre a bola e o gol de ataque
            Point2d vec_ball_atk = centroid_atk-ball_pos;
            double ang_ball_atk = angle_two_points(vec_ball_atk,eixo_x);
            if (vec_ball_atk.y < 0)
                    ang_ball_atk = -ang_ball_atk;
            //ajusta angulos para menores que 180 e maiores que -180
            if (ang_ball_atk > 180) ang_ball_atk = ang_ball_atk - 360;
            else if (ang_ball_atk < -180) ang_ball_atk = ang_ball_atk + 360;
            //cout << "Angulo bola atk: " << ang_ball_atk << endl;
            set_orientation(ang_ball_atk);
            //orientation = 45;
            //Corrige Posicionamento novamente
            ball_pos.y = -ball_pos.y;
            centroid_atk.y=-centroid_atk.y;
        }

    }else{
        //tratar a bola aqui
    }

    /*if (drible)
    {
    //Define o enimigo mais proximo
    Point2d enemy_prox;
    enemy_prox.y = -enemy_prox.y;
    if ((euclidean_dist(ball_pos,enemy_pos[0]) <= euclidean_dist(ball_pos,enemy_pos[1])) && (euclidean_dist(ball_pos,enemy_pos[0]) <= euclidean_dist(ball_pos,enemy_pos[2])))
        enemy_prox = enemy_pos[0];
    else if (euclidean_dist(ball_pos,enemy_pos[1]) <= euclidean_dist(ball_pos,enemy_pos[2]))
        enemy_prox = enemy_pos[1];
    else
        enemy_prox = enemy_pos[2];
    //Calcula o angulo entre a bola e o inimigo mais proximo
    Point2d vec_ball_enemy = enemy_prox-ball_pos;
    double ang_ball_enemy = angle_two_points(vec_ball_enemy,eixo_x);
    //Corrige o angulo
    if (vec_ball_enemy.y < 0)
            ang_ball_enemy = -ang_ball_enemy;
    //ajusta angulos para menores que 180 e maiores que -180
    if (ang_ball_enemy > 180) ang_ball_enemy = ang_ball_enemy - 360;
    else if (ang_ball_enemy < -180) ang_ball_enemy = ang_ball_enemy + 360;
    //cout << "Angulo bola enemy: " << ang_ball_enemy << endl;

    //Calcula a orientacao como base na bola, inimigo mais proximo e gol adversario
    if ((ang_ball_enemy-ang_ball_atk) > 0)
        orientation = ang_ball_atk - 90*cos((3.1415/180)*(ang_ball_enemy-ang_ball_atk))*pow(2.7183,-0.04620*euclidean_dist(ball_pos,enemy_prox));
    else
        orientation = ang_ball_atk + 90*cos((3.1415/180)*(ang_ball_enemy-ang_ball_atk))*pow(2.7183,-0.04620*euclidean_dist(ball_pos,enemy_prox));
    }*/

   // cout<<"Orientação: "<<orientation<<endl;
    while(iterator_cpo()>1E-6);
    set_direction();

    // Calcula velocidades
    Point robot_grid = convert_C_to_G(robo->get_pos());

    theta = get_direction(robot_grid);
    alpha = theta - robo->get_angle();

    if (fabs(alpha) <= limiar_theta){
        w = k*v_max*alpha/180;
        v = -v_max*fabs(alpha)/limiar_theta + v_max;
    }
    else{
        alpha = ajusta_angulo(alpha+180);
        w = k*v_max*alpha/180;
        v = v_max*fabs(alpha)/limiar_theta - v_max;
    }

    vels->first = v - w*l;
    vels->second = v + w*l;
}

void Mover::striker(Robot *robo, int num_Robo, pair<float, float> *vels){
    Point2d eixo_x(1.0,0.0);
    //if(!grid_initialized){
        init_grid();
    //}
    for(i = 0; i < 3; ++i){
        if(enemy_pos[i].x > 0 && enemy_pos[i].y > 0){
            enemy_pos_grid[i] = convert_C_to_G(enemy_pos[i]);
            //cout<<"Inimigo "<<enemy_pos_grid[i].x<<" "<<enemy_pos_grid[i].y<<endl;
            if(enemy_pos_grid[i].x>0 && enemy_pos_grid[i].y>0)
                set_potential(enemy_pos_grid[i].y, enemy_pos_grid[i].x, 1);
        }else{
            //tratar posição dos inimigos aqui
        }

        if(team_pos[i].x > 0 && team_pos[i].y > 0){
            team_pos_grid[i] = convert_C_to_G(team_pos[i]);
            //cout<<"Amigo "<<team_pos_grid[i].x<<" "<<team_pos_grid[i].y<<endl;
        }else{
            //tratar posição dos miguxos aquieuclidean_dist(ball_pos,enemy_prox
        }
    }

    if(ball_pos.x > 0 && ball_pos.y > 0){
        //Utiliza o robo amigo mais próximo para definição do epsilon
        Point2d team_prox;
        if ((euclidean_dist(ball_pos,team_pos[0]) <= euclidean_dist(ball_pos,team_pos[1])) && (euclidean_dist(ball_pos,team_pos[0]) <= euclidean_dist(ball_pos,team_pos[2])))
            team_prox = team_pos[0];
        else if (euclidean_dist(ball_pos,team_pos[1]) <= euclidean_dist(ball_pos,team_pos[2]))
            team_prox = team_pos[1];
        else
            team_prox = team_pos[2];

        set_epsilon(0.3 + euclidean_dist(team_prox,ball_pos)/250);
       // cout << " epsilon: " << e << endl;

        // Calculo do angulo de orientacao usar no ataque leve para dribles
        //Corrige Posicionamento
        ball_pos.y = -ball_pos.y;
        centroid_atk.y = -centroid_atk.y;

        //Calcula angulo entre a bola e o gol de ataque
        Point2d vec_ball_atk = centroid_atk-ball_pos;
        double ang_ball_atk = angle_two_points(vec_ball_atk,eixo_x);
        if (vec_ball_atk.y < 0)
                ang_ball_atk = -ang_ball_atk;
        //ajusta angulos para menores que 180 e maiores que -180
        if (ang_ball_atk > 180) ang_ball_atk = ang_ball_atk - 360;
        else if (ang_ball_atk < -180) ang_ball_atk = ang_ball_atk + 360;
        //cout << "Angulo bola atk: " << ang_ball_atk << endl;
        set_orientation(ang_ball_atk);
        //orientation = 45;
        //Corrige Posicionamento novamente
        ball_pos.y = -ball_pos.y;
        centroid_atk.y=-centroid_atk.y;

        Point meta;
        if (euclidean_dist(ball_pos,team_prox) < 5){
            if (centroid_atk.x > 0 && centroid_atk.y > 0){
                meta = convert_C_to_G(centroid_atk);
                set_epsilon(0);
                //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
                if (meta.x > 0 && meta.y > 0)
                    set_potential(meta.y, meta.x, 0);
            }else{
                //tratar o gol aqui
            }

        }else{
            ball_pos_grid = convert_C_to_G(ball_pos);
             //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
            if (ball_pos_grid.x > 0 && ball_pos_grid.y > 0)
                set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);
        }
    }else{
        //tratar a bola aqui
    }
    while(iterator_cpo()>1E-6);
    set_direction();

    // Calcula velocidades
    Point robot_grid = convert_C_to_G(robo->get_pos());

    theta = get_direction(robot_grid);
    alpha = theta - robo->get_angle();

    if (fabs(alpha) <= limiar_theta){
        w = k*v_max*alpha/180;
        v = -v_max*fabs(alpha)/limiar_theta + v_max;
    }
    else{
        alpha = ajusta_angulo(alpha+180);
        w = k*v_max*alpha/180;
        v = v_max*fabs(alpha)/limiar_theta - v_max;
    }

    vels->first = v - w*l;
    vels->second = v + w*l;
}
