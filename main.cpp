#include <iostream>
#include <bits/stdc++.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
using namespace std;

struct Bat
{
    int id = 10;
    string state;
    int right_state = 0;
    int left_state;
    int current_state;
};



class Monitor_Batman
{
    pthread_mutex_t monitor;
    pthread_mutex_t lock_area;
    pthread_cond_t all_directions[4];
    pthread_cond_t same_state[4];
    // number of bats in each direction
    // 0 -> north
    // 1 -> east
    // 2 -> south
    // 3 -> west
    int count_elements [4];
    // waiting bats in each direction for a lock
    bool waiters[4];
public :
    Monitor_Batman()
    {

        monitor = PTHREAD_MUTEX_INITIALIZER;

        lock_area = PTHREAD_MUTEX_INITIALIZER;

        //initialize all condition variables
        for(int j = 0; j< 4; j++)
        {
            all_directions[j] = PTHREAD_COND_INITIALIZER;
            same_state[j] = PTHREAD_COND_INITIALIZER;
            waiters[j] = false;
            count_elements[j] = 0;
        }
    }

public :
    void arrive (Bat b)
    {
        pthread_mutex_lock(&monitor);
        count_elements[b.current_state] ++;
        if(count_elements[b.current_state] > 1)
        {
            pthread_cond_wait(&same_state[b.current_state], &monitor);
        }
        // print arrived
        cout << "BAT " << b.id << " from " << b.state << " arrives at crossing" <<endl;
        pthread_mutex_unlock(&monitor);

    }


public :
    void cross(Bat b)
    {
        pthread_mutex_lock(&monitor);
        if(count_elements[b.right_state] > 0)
        {
            waiters[b.current_state] = true;
            pthread_cond_wait(&all_directions[b.right_state], &monitor);
            waiters[b.current_state]  = false;
        }
        pthread_mutex_lock(&lock_area);
        cout << "BAT " << b.id << " from " << b.state << " crossing" <<endl;
        pthread_mutex_unlock(&monitor);
        sleep(1);
        pthread_mutex_unlock(&lock_area);
    }

public :
    void leave(Bat b)
    {
        pthread_mutex_lock(&monitor);
        count_elements[b.current_state] -- ;
        cout << "BAT " << b.id << " from " << b.state << " leaving crossing" <<endl;
        if(waiters[b.left_state])
        {
            pthread_cond_signal(&all_directions[b.current_state]);
        }
        pthread_cond_signal(&same_state[b.current_state]);
        pthread_mutex_unlock(&monitor);

    }

    public : void check()
    {
        pthread_mutex_lock(&monitor);
        if(waiters[0] && waiters[1] && waiters[2] && waiters[3]){
            cout << "DEADLOCK: BAT jam detected, signalling North Direction to go" << endl;
            pthread_cond_signal(&all_directions[0]);
        }
        pthread_mutex_unlock(&monitor);
    }


    public : void destroy () {
    pthread_mutex_destroy(&monitor);

    pthread_mutex_destroy(&lock_area);

    pthread_cond_destroy(&all_directions[0]);
    pthread_cond_destroy(&all_directions[1]);
    pthread_cond_destroy(&all_directions[2]);
    pthread_cond_destroy(&all_directions[3]);

    pthread_cond_destroy(&same_state[0]);
    pthread_cond_destroy(&same_state[1]);
    pthread_cond_destroy(&same_state[2]);
    pthread_cond_destroy(&same_state[3]);

}

};

Monitor_Batman m = Monitor_Batman();

void* task(void* args)
{
    Bat *b = (Bat*)args;
    Bat bat;
    bat.current_state = b->current_state;
    bat.id = b->id;
    bat.left_state = b->left_state;
    bat.right_state = b->right_state;
    bat.state = b->state;
    m.arrive(bat);
    m.cross(bat);
    m.leave(bat);
}

void *batTask (void* id) {
    while (true) {
        m.check();
        sleep(2);
    }
}


int main()
{
    cout << "enter the direction of bats :";
    string directions;
    cin >> directions;
    pthread_t bats[directions.length()];
    pthread_t batManThread;
    Bat all_bats[directions.length()];

    if (pthread_create(&batManThread, NULL, batTask, NULL) > 0) {
        cout << "Failed to create thread" << endl;
    }

    for(int i = 0 ; i < directions.length() ; i++)
    {
        if(directions.at(i) == 'n'){
            all_bats[i].id = i+1;
            all_bats[i].current_state = 0;
            all_bats[i].left_state = 1;
            all_bats[i].right_state = 3;
            all_bats[i].state = "Nourth";
        } else if(directions.at(i) == 'e'){
            all_bats[i].id = i+1;
            all_bats[i].current_state = 1;
            all_bats[i].left_state = 2;
            all_bats[i].right_state = 0;
            all_bats[i].state = "East";
        }else if(directions.at(i) == 's'){
            all_bats[i].id = i+1;
            all_bats[i].current_state = 2;
            all_bats[i].left_state = 3;
            all_bats[i].right_state = 1;
            all_bats[i].state = "SOUTH";
        }else if(directions.at(i) == 'w'){
            all_bats[i].id = i+1;
            all_bats[i].current_state = 3;
            all_bats[i].left_state = 0;
            all_bats[i].right_state = 2;
            all_bats[i].state = "WEST";
        }

        if(pthread_create(&bats[i],NULL,task,(void*)&all_bats[i]) > 0)
            cout << "failed";
    }

    for(int i = 0 ;i < directions.length() ; i++)
    {
        pthread_join(bats[i] , NULL);
    }
    m.destroy();

    return 0;
}

