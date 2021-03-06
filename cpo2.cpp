#include <iostream>
#include <iomanip>
#include <cmath>
#include "math.h"
#include "cpo2.h"
#include "utils.h"
#include "robot.h"

using namespace std;



CPO2::CPO2(){

    stop = true;
    grid_initialized = false;
    enemy_pos_grid = pVector(3);
    team_pos_grid = pVector(3);
    pGrid = dMatrix(28, vector<double>(36, 0.0));
    tGrid = dMatrix(28, vector<double>(36, 0));
    cout<<"\n\nAMBIENTE CRIADO!\n";
}

double CPO2::iterator(){

    double erro = 0;
    double top, botton, left, right;
    double newPotencial, oldPotencial;
    double vec[2], e, h, lambda;
    int i,j;

    vec[0] = cos(orientation*PI/180);
    vec[1] = sin(orientation*PI/180);

    h = dx/dy;

    e = 0.8;
    lambda = e*h/2;

     for(i=0;i<28;i++)
     {
         for(j=0;j<36;j++)
         {
             if(get_occupancy(i,j)==1)
             {
                oldPotencial = get_potential(i,j);
                top = get_neighborhood(i,j,0);
                 botton = get_neighborhood(i,j,1);
                 left = get_neighborhood(i,j,2);
                 right = get_neighborhood(i,j,3);
                 newPotencial = ((1+lambda*vec[0])*right+(1-lambda*vec[0])*left+(1+lambda*vec[1])*top+(1-lambda*vec[1])*botton)/4;
                 newPotencial = newPotencial + 0.8*(newPotencial-oldPotencial);
                 erro = erro + pow((newPotencial - oldPotencial),2);
                 set_potential(i,j,newPotencial);
             }
         }
     }
     return erro;
}

double CPO2::get_neighborhood(int i, int j, int k){
    double top,botton,left,right;
    if(i==0)
    {
        top=1;
        botton=get_potential(i+1,j);
    }
    else
    {
        if(i==27)
        {
            botton=1;
            top=get_potential(i-1,j);
        }
        else
        {
            top=get_potential(i-1,j);
            botton=get_potential(i+1,j);
        }
    }
    if(j==0)
    {
        left=1;
        right=get_potential(i,j+1);
    }
    else
    {
        if(j==35)
        {
            right=1;
            left=get_potential(i,j-1);
        }
        else
        {
            left=get_potential(i,j-1);
            right=get_potential(i,j+1);
        }
    }
    switch (k) {
    case 0:
        return top;
        break;
    case 1:
        return botton;
        break;
    case 2:
        return left;
        break;
    case 3:
        return right;
        break;
    default:
        break;
    }
 }


int CPO2::get_occupancy(int i, int j){
    if(get_potential(i,j)==1 || get_potential(i,j)==0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

double CPO2::get_potential(int i, int j){
    return pGrid[i][j];
}

void CPO2::set_direction(){
    int i,j;

    for(i=0;i<28;i++)
    {
        for(j=0;j<36;j++)
        {
            //cout << "i = " << i << " j = " << j << endl;
            if(get_occupancy(i,j)==1)
                tGrid[i][j] = -atan2(get_neighborhood(i,j,0)-get_neighborhood(i,j,1),get_neighborhood(i,j,2)-get_neighborhood(i,j,3))*180/PI;
            else
                tGrid[i][j] = get_potential(i,j)*1000;
        }
    }
}

double CPO2::get_direction(Point grid){
    return this->tGrid[grid.y][grid.x];;
}

void CPO2::set_potential(int i, int j, double aux){
    pGrid[i][j]= aux;
}

void CPO2::init_grid(){
    int i,j;

    for(i=0;i<28;i++)
    {
        for(j=0;j<36;j++)
        {
            if ( i == 0 || i == 27 || j == 0 || j == 1 || j == 2 || j == 33 || j == 34 || j == 35 )
            {
                if ( j == 1 || j == 2 || j == 33 || j == 34 )
                {
                    if (i > 9 && i < 18)
                        pGrid[i][j] = 0.9;
                    else
                        pGrid[i][j] = 1.0;
                }
                else
                    pGrid[i][j] = 1.0;
            }
            else
                pGrid[i][j] = 0.9;
        }

    }
}

void CPO2::print_grid(){
    cout<<"\n\n\nGrid:\n\n";;
    int i,j;
    for(i=0;i<28;i++)
    {
        for(j=0;j<36;j++)
        {
            cout.precision(4);
            cout<<fixed;
            cout<<pGrid[i][j]<<setw(7);
        }
        cout<<"\n\n";
    }
    cout<<"\n\n\nGrid de direcoes:\n\n";
    for(i=0;i<28;i++)
    {
        for(j=0;j<36;j++)
        {
            cout<<tGrid[i][j]<<setw(7);
        }
        cout<<"\n\n";
    }
    cout<<"\n\n\n";
}

Point CPO2::convert_C_to_G(Point2d coord){
    Point i;

    coord.x = int(coord.x) + 5;
    coord.y = int(coord.y) + 5;

    if(coord.x / dx != 35){
        i.x = coord.x / dx;
    }else{
        i.x = coord.x / dx - 1;
    }

    if(coord.y / dy != 27){
        i.y = coord.y / dy;
    }else{
        i.y = coord.y / dy - 1;
    }
    //cout << "i.x = " << i.x << " i.y = " << i.y << endl;
    return i;
}


void CPO2::run(){
    Point2d eixo_x(1.0,0.0);
    int i;
    if(!grid_initialized){
        init_grid();
        grid_initialized = true;
    }
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
        ball_pos_grid = convert_C_to_G(ball_pos);
         //cout<<"Bola "<<ball_pos_grid.x<<" "<<ball_pos_grid.y<<endl;
        if (ball_pos_grid.x > 0 && ball_pos_grid.y > 0)
            set_potential(ball_pos_grid.y, ball_pos_grid.x, 0);

        //Calcula angulo entre a bola e o gol de ataque
        Point2d vec_ball_atk = centroid_atk-ball_pos;
        double ang_ball_atk = angle_two_points(vec_ball_atk,eixo_x);
        if (vec_ball_atk.y < 0)
                ang_ball_atk = -ang_ball_atk;
        //ajusta angulos para menores que 180 e maiores que -180
        if (ang_ball_atk > 180) ang_ball_atk = ang_ball_atk - 360;
        else if (ang_ball_atk < -180) ang_ball_atk = ang_ball_atk + 360;
        //cout << "Angulo bola atk: " << ang_ball_atk << endl;
        orientation = ang_ball_atk;

        //Corrige Posicionamento novamente
        ball_pos.y = -ball_pos.y;
        centroid_atk.y=-centroid_atk.y;
    }else{
        //tratar a bola aqui
    }

    // Calculo do angulo de orientacao usar no ataque leve para dribles
    //Corrige Posicionamento
    ball_pos.y = -ball_pos.y;
    centroid_atk.y = -centroid_atk.y;


   // cout << "Angulo de orientacao: " << orientation << endl;

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
    while(iterator()>1E-6);
    set_direction();

    flag_finish_CPO2 = true;
}

bool CPO2::get_flag_finish(){
    return this->flag_finish_CPO2;
}

void CPO2::zera_flag_finish(){
    flag_finish_CPO2 = false;
}

void CPO2::set_orientation(double angle){
    this->orientation = angle;
}

void CPO2::set_enemy_pos(p2dVector enemy_pos){
    this->enemy_pos = enemy_pos;
}

void CPO2::set_team_pos(p2dVector team_pos){
    this->team_pos = team_pos;
}

void CPO2::set_ball_pos(Point2d ball_pos){
    this->ball_pos = ball_pos;
}

void CPO2::set_centroid_atk(Point2d centroid_atk){
    this->centroid_atk = centroid_atk;
}

void CPO2::set_centroid_def(Point2d centroid_def){
    this->centroid_def = centroid_def;
}

bool CPO2::isStopped() const
{
    return this->stop;
}

void CPO2::Play(){
    if(!isRunning()){
        if(isStopped())
            stop = false;
        start(LowPriority);
    }
}

void CPO2::Stop(){
    stop = true;
}

bool CPO2::is_running(){
    return isRunning();
}

void CPO2::msleep(int ms){
    /*struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);*/
}

CPO2::~CPO2(){

}
