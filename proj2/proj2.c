#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <string.h>

//global variables
int *oxygen = 0;
int *hydrogen = 0;
int *i_log = 0;
int *count = 0;
int *molecule_count = 0;
int *check_NN = 0;
int *expect_molecules = 0;
int *flag = 0;

int oxygen_id;
int hydrogen_id;
int i_log_id;
int count_id;
int molecule_count_id;
int check_NN_id;
int expect_molecules_id;
int flag_id;

sem_t *hydroQueue;
sem_t *oxyQueue;
sem_t *mutex;
sem_t *mutexProcc;
sem_t *get_i_log;
sem_t *turn1;
sem_t *turn2;

FILE *f;


//functions 
int number_check(char *number);
void init_sem();
void init_memory();
void kill_sem();
void kill_memory();
void shm_err(int x);
void oxyProcc(int TI, int TB, int i, int NO, int NH);
void HydroProcc(int TI, int i, int NO, int NH);
void write_log(char *name, int i, char *text, int n, int x, int NO, int NH);

// init all semaphores
// if init fails error will display
void init_sem(){
    if ((hydroQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(hydroQueue, 1, 0) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if ((oxyQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(oxyQueue, 1, 0) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if ((mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(mutex, 1, 1) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if ((get_i_log = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(get_i_log, 1, 1) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if ((mutexProcc = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(mutexProcc, 1, 1) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if ((turn1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(turn1, 1, 0) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if ((turn2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
    if (sem_init(turn2, 1, 1) == -1){
        fprintf(stderr, "Error when creating semaphore\n");
        exit(1);
    }
}

// init all memory
//  if init fails error will display
void init_memory()
{
    if ((i_log_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(3);
    if ((i_log = (int *)shmat(i_log_id, NULL, 0)) == (void *)-1)
        shm_err(4);

    if ((oxygen_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(7);
    if ((oxygen = (int *)shmat(oxygen_id, NULL, 0)) == (void *)-1)
        shm_err(8);

    if ((hydrogen_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(9);
    if ((hydrogen = (int *)shmat(hydrogen_id, NULL, 0)) == (void *)-1)
        shm_err(10);

    if ((count_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(99);
    if ((count = (int *)shmat(count_id, NULL, 0)) == (void *)-1)
        shm_err(98);

    if ((molecule_count_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(96);
    if ((molecule_count = (int *)shmat(molecule_count_id, NULL, 0)) == (void *)-1)
        shm_err(95);

    if ((check_NN_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(80);
    if ((check_NN = (int *)shmat(check_NN_id, NULL, 0)) == (void *)-1)
        shm_err(81);

    if ((expect_molecules_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(55);
    if ((expect_molecules = (int *)shmat(expect_molecules_id, NULL, 0)) == (void *)-1)
        shm_err(54);
    if ((flag_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        shm_err(30);
    if ((flag = (int *)shmat(flag_id, NULL, 0)) == (void *)-1)
        shm_err(31);
}

// destroy all semaphores and memory
// if destroing fails error will display
void kill_sem()
{
    if (sem_destroy(hydroQueue) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
    if (sem_destroy(oxyQueue) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
    if (sem_destroy(mutex) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
    if (sem_destroy(get_i_log) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
    if (sem_destroy(mutexProcc) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
    if (sem_destroy(turn1) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
    if (sem_destroy(turn2) == -1){
        fprintf(stderr, "Error when destroying semaphore\n");
        exit(1);
    }
}
// destroyng memory
// if destroing fails error will display
void kill_memory(){

    shmdt(i_log);
    shmdt(oxygen);
    shmdt(hydrogen);
    shmdt(count);
    shmdt(molecule_count);
    shmdt(check_NN);
    shmdt(expect_molecules);
    shmdt(flag);

    if (shmctl(i_log_id, IPC_RMID, NULL) == -1)
        shm_err(12);
    if (shmctl(oxygen_id, IPC_RMID, NULL) == -1)
        shm_err(14);
    if (shmctl(hydrogen_id, IPC_RMID, NULL) == -1)
        shm_err(15);
    if (shmctl(count_id, IPC_RMID, NULL) == -1)
        shm_err(97);
    if (shmctl(molecule_count_id, IPC_RMID, NULL) == -1)
        shm_err(94);
    if (shmctl(check_NN_id, IPC_RMID, NULL) == -1)
        shm_err(50);
    if (shmctl(expect_molecules_id, IPC_RMID, NULL) == -1)
        shm_err(59);
    if (shmctl(flag_id, IPC_RMID, NULL) == -1)
        shm_err(29);
}
//simple print function for debug of memory problems (prints number of error)
void shm_err(int x)
{
    fprintf(stderr, "Error when init of shared memory. %d\n", x);
    exit(1);
}

// WRITE (main write function )
// i oxygen/hydrogen, n molecule,
// NAME H/O
// text what is happening
//int n and int x used more like flags 
void write_log(char *name, int i, char *text, int n, int x,int NO, int NH)
{
    sem_wait(get_i_log);
    if (i == -1 && n == -1){
        fprintf(f, "%d: %s %d: %s %d\n", ++(*i_log), name, ++i, text, ++n); //
    }
    else if (n == -1){
        fprintf(f, "%d: %s %d: %s\n", ++(*i_log), name, ++i, text); //
    }
    else if (x == 1){
        *check_NN = *check_NN + 1;
        fprintf(f, "%d: %s %d: molecule %d %s\n", ++(*i_log), name, ++i, ++n, text); //
        if (*check_NN == 3)
        {
            //increment molecule count if all 3 (2*H and 1*O)did print 
            *molecule_count = *molecule_count + 1;
            *check_NN = 0;
            // help if to look if there is enough H or O
            if (*expect_molecules == *molecule_count){
                if ((*oxygen == NO - *molecule_count)&&(*hydrogen == NH - (*molecule_count*2))){
                    sem_post(oxyQueue);
                    sem_post(hydroQueue);
                }

            }
        }
    }
    else if (x == 3){
        sem_post(oxyQueue);
        fprintf(f, "%d: %s %d: %s\n", ++(*i_log), name, ++i, text);
    }
    else if (x == 4){
        sem_post(hydroQueue);
        fprintf(f, "%d: %s %d: %s\n", ++(*i_log), name, ++i, text);
    }
    else if (x == 2){
        fprintf(f, "%d: %s %d: %s %d\n", ++(*i_log), name, ++i, text, ++n); //
    }

    fflush(NULL);
    sem_post(get_i_log);
}

// check if value is number or if if NULL entered

int number_check(char *number){
    char *other = {'\0'};
    int result = 0;
    result = (int)strtol(number, &other, 10);
    //used strcmp for looking for NULL to differ it from zero
    int tmp = strcmp(number, "");
    if (tmp == 0){
        fprintf(stderr, "Invalid input\n");
        exit(1);
    }
    if (*other != '\0'){
        fprintf(stderr, "Invalid input\n");
        exit(1);
    }

    return (result);
}
//oxygen procc
void oxyProcc(int TI, int TB, int i, int NO, int NH)
{

    sem_wait(mutex);
    // print start
    write_log("O", i, "started", -1, 0, NO, NH);

    sem_post(mutex); // randomize it more
    sem_wait(mutex); 

    // usleep if 0 skip and dont use usleep
    if (TI != 0){
        usleep(rand() % TI);
    }

    // going to quete
    write_log("O", i, "going to queue", -1, 0, NO, NH);

    *oxygen = *oxygen + 1;
    if (*expect_molecules == *molecule_count)
    {
        if ((*oxygen == NO - *molecule_count) && (*hydrogen == NH - (*molecule_count * 2)))
        {
            sem_post(oxyQueue);
            sem_post(hydroQueue);
        }
    }
    if (*hydrogen >= 2){
        sem_post(hydroQueue);
        sem_post(hydroQueue);
        *hydrogen = *hydrogen - 1;
        *hydrogen = *hydrogen - 1;
        sem_post(oxyQueue);
        *oxygen = *oxygen - 1;
    }
    else{
        sem_post(mutex);
    }


    sem_wait(oxyQueue);
    // help if to look if there is enough H or O
    if (*expect_molecules == *molecule_count)
    {
        *hydrogen = *hydrogen - 1;
        write_log("O", i, "not enough H", 0, 3, NO, NH);
        exit(0);
    }

    // bond
    write_log("O", i, "creating molecule", *molecule_count, 2, NO, NH);

    // barrier
    sem_wait(mutexProcc);
    *count = *count + 1;
    if (*count == 3){
        sem_wait(turn2);
        sem_post(turn1);
    }
    sem_post(mutexProcc);
    sem_wait(turn1);
    sem_post(turn1);

    // usleep if 0 skip and dont use usleep
    if (TB != 0){
        usleep(rand() % TB );
    }

    //write created molecule
    write_log("O", i, "created", *molecule_count, 1, NO, NH);
    sem_wait(mutexProcc);
    *count = *count - 1;
    if (*count == 0){
        sem_wait(turn1);
        sem_post(turn2);
    }
    sem_post(mutexProcc);
    sem_wait(turn2);
    sem_post(turn2);

    //finish
    sem_post(mutex);
    exit(0);
}

void hydroProcc(int TI, int i, int NO, int NH)
{
    sem_wait(mutex);

    // write
    write_log("H", i, "started", -1, 0, NO, NH);

    sem_post(mutex);        //TODO
    sem_wait(mutex);        //TODO

    // usleep if 0 skip and dont use usleep
    if (TI != 0){
        usleep(rand() % TI);
    }

    // write
    write_log("H", i, "going to queue", -1, 0, NO, NH);

    *hydrogen = *hydrogen + 1;
    if (*expect_molecules == *molecule_count)
    {
        if ((*oxygen == NO - *molecule_count) && (*hydrogen == NH - (*molecule_count * 2)))
        {
            sem_post(oxyQueue);
            sem_post(hydroQueue);
        }
    }
    if ((*hydrogen >= 2) && (*oxygen >= 1)){
        sem_post(hydroQueue);
        sem_post(hydroQueue);
        *hydrogen = *hydrogen - 1;
        *hydrogen = *hydrogen - 1;
        sem_post(oxyQueue);
        *oxygen = *oxygen - 1;
    }
    else{
        sem_post(mutex);
    }
    
    sem_wait(hydroQueue);
    // help if to look if there is enough H or O
    if (*expect_molecules == *molecule_count)
    {
        *oxygen = *oxygen - 1;
        write_log("H", i, "not enough O or H", 0, 4, NO, NH);
        exit(0);
    }

    //  bond
    write_log("H", i, "creating molecule", *molecule_count, 2,NO, NH);

    // barrier
    sem_wait(mutexProcc);
    *count = *count + 1;
    if (*count == 3){
        sem_wait(turn2);
        sem_post(turn1);
    }
    sem_post(mutexProcc);
    sem_wait(turn1);
    sem_post(turn1);

    // molecula created
    write_log("H", i, "created", *molecule_count, 1, NO, NH);

    sem_wait(mutexProcc);
    *count = *count - 1;
    if (*count == 0){
        sem_wait(turn1);
        sem_post(turn2);
    }
    sem_post(mutexProcc);
    sem_wait(turn2);
    sem_post(turn2);

    //finish
    exit(0);
}

int main(int argc, char *argv[])
{
    //  argument check
    if (argc != 5)
    {
        fprintf(stderr, "Error: Wrong numer of arguments\n");
        exit(1);
    }

    int NO = number_check(argv[1]); // number of oxygen
    int NH = number_check(argv[2]); // number of hydrogen
    // slepp in microseconds theyfore *1000
    int TI = (number_check(argv[3])) * 1000; // max time of H2O wait
    int TB = (number_check(argv[4])) * 1000; // max time of creation of one molecule
    // check of arg range
    if (!(NO > 0) || !(NH > 0) || !(TI >= 0) || !(TI <= 1000000) || !(TB >= 0) || !(TB <= 1000000)){
        fprintf(stderr, "Error: wrong value, check range of values\n");
        exit(1);
    }
    // open file for LOG
    f = fopen("proj2.out", "w");
    if (f == NULL){
        fprintf(stderr, "Error: Could not open the file\n");
        exit(1);
    }
    pid_t wpid;
    // init
    init_sem();
    init_memory();

    double tmp = 0;
    // expect molecule count
    tmp = (double)NH / (double)NO;
    if (tmp > (double)2){
        *expect_molecules = NO;
    }
    else if (tmp < (double)2){
        *expect_molecules = (NH / 2);
    }
    else{
        *expect_molecules = (NH / 2);
    }

    // oxygen process
    pid_t pid_O[NO];
    for (int i = 0; i < NO; i++)
    {
        time_t t;
        srand((unsigned)time(&t));
        pid_O[i] = fork();
        if (pid_O[i] == 0){
            oxyProcc(TI, TB, i, NO, NH);
        }
        else if (pid_O[i] == -1){
            fprintf(stderr, "Error could not fork\n");
            exit(1);
        }
    }

    // hydrogen process
    pid_t pid_H[NH];
    for (int i = 0; i < NH; i++)
    {
        time_t t;
        srand((unsigned)time(&t));

        pid_H[i] = fork();
        if (pid_H[i] == 0){
            hydroProcc(TI, i, NO, NH);
        }
        else if (pid_H[i] == -1){
            fprintf(stderr, "Error could not fork\n");
            exit(1);
        }
    }
    // wait for child

    for (int i = 0; i < NO; i++){
        while ((wpid = wait(&pid_O[i])) > 0);
    }
    for (int i = 0; i < NH; i++){
        while ((wpid = wait(&pid_H[i])) > 0);
    }

    // destroy
    kill_sem();
    kill_memory();

    //close file
    fclose(f);

    return 0;
}
