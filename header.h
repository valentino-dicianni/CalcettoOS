#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <signal.h>



#define MSGIDFATE 5253
#define IDFATE 9929
#define CONFIGPATH "../../config"
#define TEAM0 "../../team0"
#define TEAM1 "../../team1"

#define SEMID 7389
#define SEMIDTEAM 73202

//Struct zone

typedef struct{
    int team0;
    int team1;
}Score;

typedef struct{
    long mtype;
    int pid;
    int action;
}msgToFate;

typedef struct{
    long mtype;
    int res;
    int pid;
}fateResponse;

struct sigaction sigAction,sigActionDef;


//arbitro
void createFato();
void createSquadre();

//Squadra
void createGiocatori(int playerNumber, int teamNumber, char*nameTeam);

//Giocatore
int play(int* action, char** name);

//Fato
void setupLogFile(char **nomeDirectoryLog);
void writeToLog(char *text);
int askFate(double probability);
float getProbability(int action);
char* statementGenerator(int action, int response, int isSamePlayer);

//Common
extern float getConfigValue(char *configOption, char *path);
char* getTeamSpecific(char *option, char *path);
int initSemAvailable(int semId, int semNum);
int customInitSemAvailable(int semId, int semNum, int init);
int reserveSem(int semId,int semNum);
int releaseSem(int semId,int semNum);
void sigHandler(int sig);

//Manager
void showGuide();
void editConf();
void showMenu();

#endif //CALCETTOOS_CONFIGFILE_H
