#include <string.h>
#include <sys/wait.h>
#include "header.h"

int volatile flagSignal = 0;
int main(int argc, char** arcgv){
    int semIDTeam;
    int teamNumber;
    char* nameTeam;
    int status;
    int count=0;

    /* RECUPERO INFO SQUADRA */
    teamNumber = *(arcgv[0]);
    if(teamNumber == 0)
        nameTeam =  getTeamSpecific("name",TEAM0);
    else
        nameTeam =  getTeamSpecific("name",TEAM1);

    printf("[SQUADRA_%s] Processo creato SQUADRA: %s\n",nameTeam,nameTeam);


    /* RECUPERO INFO SEMAFORO SQUADRA */
    if((semIDTeam=semget(SEMIDTEAM,2,0666)) == -1)
        printf("[SQUADRA_%s]**ERRORE** nel recupero del semaforo del team\n",nameTeam);
    int c = 0;

    /* SEGNALE SIGALARM CHE ARRIVA DA ARBITRO A FINE PARTITA */
    sigAction.sa_handler = &sigHandler;
    sigAction.sa_flags = SA_RESTART;
    sigaction (SIGALRM, NULL, &sigActionDef);
    if (sigActionDef.sa_handler != SIG_IGN)
        sigaction (SIGALRM, &sigAction, NULL);

    /* CICLO DI ESECUZIONE, FINO A QUANDO NON VIENE RILEVATO UN SEGNALE DI FINE PARTITA */
    while(flagSignal==0){
        while(c<5) {
            reserveSem(semIDTeam, teamNumber);
            createGiocatori(c, teamNumber, nameTeam);
            c++;
        }
        reserveSem(semIDTeam, teamNumber);
        //Il codice seguente nel ciclo viene eseguito solo quando un giocatore dei 5 esce dal campo
        if(flagSignal==1) {
            /* RECUPERO PROCESSI FIGLI (I VARI GIOCATORI) */
            while ((waitpid(-1, &status, 0)) > 0) {
                fflush(stdout);
                count++;
            }
            printf("[SQUADRA_%s] Sono rientrati negli spogliatoi %d giocatori.\n",nameTeam,count);
            break;
        }
        else {
            if(flagSignal==0 && waitpid(-1, &status, 0)>0) {
                createGiocatori(c, teamNumber, nameTeam);
                c++;
            }
        }
    }
    return 0;
}

void sigHandler(int sig) {
    if(sig == SIGALRM ){
        flagSignal=1;
    }
}

void createGiocatori(int  playerNumber, int teamNumber, char* nameTeam) {

    char *numTeam = malloc(sizeof(int));
    sprintf(numTeam, "%d", teamNumber);

    char *numPlay = malloc(sizeof(char)*512);
    sprintf(numPlay, "%d", playerNumber);

    char *argv[] = {numTeam, numPlay, NULL};
    char *envp[] = { NULL };

    pid_t i = fork();

    if (flagSignal==0) {
        if (i == 0) {
            execve("./Giocatore", argv, envp);
            exit(EXIT_FAILURE);
        } else if (i > 0) {
            printf("[SQUADRA_%s] - Scende in campo un nuovo giocatore con pid %d\n", nameTeam, i);
        } else {
            perror("[SQUADRA]**ERRORE** Fork fallito\n");
        }
    }

    free(numPlay);
    free(numTeam);
}