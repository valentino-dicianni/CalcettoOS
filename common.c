#include <memory.h>
#include <errno.h>
#include "header.h"

#ifdef _SEM_SEMUN_UNDEFINED
union semun{
    int val; /* Valore per SETVAL */
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};
#endif

float getConfigValue(char *configOption, char *path){
    float configValue=0;
    int i=0;
    char *rowConfigOption =malloc(sizeof(char)*50);

    FILE *fp = fopen(path, "r");
    if( fp == NULL ) {
        printf("Errore all'apertura del file di configurazione: %s \n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (fscanf(fp, "%s %f", rowConfigOption, &configValue)!=EOF){
        if(strcmp(configOption,rowConfigOption)==0){
            fclose(fp);
            free(rowConfigOption);
            return configValue;
        }
        i++;
    }
    fclose(fp);
    free(rowConfigOption);
    return -1;
}

char* getTeamSpecific(char *option, char *path){
    int i=0;
    char* name=malloc(sizeof(char)*50);
    char* surname=malloc(sizeof(char)*50);
    char *rowConfigOption =malloc(sizeof(char)*50);

    FILE *fp = fopen(path, "r");
    if( fp == NULL ) {
        printf("Errore all'apertura del file delle squadre: %s \n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (fscanf(fp, "%s %s %s", rowConfigOption, name,surname)!=EOF){
        if(strcmp(option,rowConfigOption)==0){
            fclose(fp);
            free(rowConfigOption);
            strcat(name," ");
            strcat(name,surname);
            return name;
        }
        i++;
    }
    fclose(fp);
    free(rowConfigOption);
    free(name);
    free(surname);
    return "error";
}

int initSemAvailable(int semId, int semNum) {
    union semun arg;
    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}

int customInitSemAvailable(int semId, int semNum, int init) {
    union semun arg;
    arg.val = init;
    return semctl(semId, semNum, SETVAL, arg);
}

int reserveSem(int semId,int semNum) {
    struct sembuf sops;
    sops.sem_num = (unsigned short) semNum;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    return semop(semId, &sops, 1);
}

int releaseSem(int semId,int semNum) {
    struct sembuf sops;
    sops.sem_num = (unsigned short) semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(semId, &sops, 1);
}
