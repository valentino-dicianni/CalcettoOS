#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include "header.h"
#ifdef _SEM_SEMUN_UNDEFINED
union semun{
    int val; /* Valore per SETVAL */
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};
#endif

int volatile flagSignal = 0;
int main(int argc,char** arcgv) {
    int semID;
    int semIDteam;
    int response;
    int randomAction,isDribbling=0;
    int teamNumber;
    int playNumber;
    char* playerName;

    /* RECUPERO INFO GIOCATORE */
    teamNumber = atoi(arcgv[0]);
    playNumber = atoi(arcgv[1])+1;
    char* temp = malloc(sizeof(char)*500);
    sprintf(temp, "%d", playNumber);

    if(teamNumber == 0)
        playerName =  getTeamSpecific(temp, TEAM0);
    else
        playerName =  getTeamSpecific(temp, TEAM1);

    free(temp);
    srand((unsigned int) (getpid() + time(NULL)));

    /* RECUPERO INFO SUL SEMAFORO PALLONE E SQUADRA */
    if((semID=semget(SEMID,1,0666)) == -1)
        printf("[GIOCATORE %s]**ERRORE** nel recupero del semaforo\n",playerName);

    if((semIDteam=semget(SEMIDTEAM,2,0666)) == -1)
        printf("[GIOCATORE %s]**ERRORE** nel recupero del semaforo del team\n",playerName);

    /* SEGNALE SIGALARM CHE ARRIVA DA ARBITRO A FINE PARTITA */
    sigAction.sa_handler = &sigHandler;
    sigAction.sa_flags = 0;
    sigaction (SIGALRM, NULL, &sigActionDef);
    if (sigActionDef.sa_handler != SIG_IGN)
        sigaction (SIGALRM, &sigAction, NULL);

    fflush(stdout);

    //Dopo una sostituzione, viene rilasciato il pallone dal nuovo giocatore che entra in campo
    if(playNumber>5)
        releaseSem(semID,0);

    /* CICLO DI ESECUZIONE, FINO A QUANDO NON VIENE RILEVATO UN SEGNALE DI FINE PARTITA */
    while(flagSignal==0) {
        reserveSem(semID,0);

        //Ricevuto un segnale di termine partita
        if(flagSignal==1){break;}

        fflush((stdout));
        printf("\n\n");
        response = play(&randomAction, &playerName);
        do {
            isDribbling=0;
            if (response==1) {
                switch (randomAction) {
                    case 1:
                        if (teamNumber == 0) {
                            if (kill(getpgid(0), SIGUSR1) == -1)
                                printf("[GIOCATORE %s]**ERRORE** nell'invio del segnale\n", playerName);
                        } else {
                            if (kill(getpgid(0), SIGUSR2) == -1)
                                printf("[GIOCATORE %s]**ERRORE** nell'invio del segnale\n", playerName);
                        }
                        printf("[GIOCATORE_%s]Ha segnato un goal fantastico!\n", playerName);
                        break;
                    case 2:
                        printf("[GIOCATORE_%s]Si è infortunato e non potrà continuare a giocare\n", playerName);
                        if(flagSignal==0 ) {
                            releaseSem(semIDteam, teamNumber);
                            flagSignal=2;
                        }
                        fflush(stdout);
                        break;
                    case 3:
                        printf("[GIOCATORE_%s]Ha driblato con successo un'altro giocatore\n", playerName);
                        printf("\n\n");
                        isDribbling =1;
                        response = play(&randomAction, &playerName);
                        fflush(stdout);
                        break;
                    default:
                        printf("[GIOCATORE_%s]**ERRORE** nella gestione dei casi delle azioni: response == 1\n", playerName);
                        break;
                }
            }
            else if(response == 0){
                switch (randomAction){
                    case 1:
                        printf("[GIOCATORE_%s]Prova il tiro in porta, ma manca il bersaglio\n", playerName);
                        break;
                    case 2:
                        printf("[GIOCATORE_%s]Si rialza dopo lo contro. Nulla di grave, può continuare a giocare\n", playerName);
                        break;
                    case 3:
                        printf("[GIOCATORE_%s]Prova il dribbling, ma fallisce miseramente\n", playerName);
                        break;
                    default:
                        printf("[GIOCATORE_%s]**ERRORE** nella gestione dei casi delle azioni response == 0\n", playerName);
                        break;
                }
            }
            else if(response==-1){ printf("[GIOCATORE_%s]Fuori tempo massimo...Azione annullata\n", playerName); }
            else{ printf("[GIOCATORE_%s]**ERRORE** nella gestione dei casi delle azioni response\n", playerName); }

        } while (isDribbling==1 && flagSignal == 0);

        if(flagSignal==0) {
            sleep(2); //simulazione della giocata
            releaseSem(semID, 0);
            //per non far riservare il pallone allo stesso giocatore
            sleep(1);
        }
    }

    printf("[GIOCATORE_%s] Fine partita, esco dal campo.\n",playerName);
    return 0;
}

void sigHandler(int sig) {
    if(sig == SIGALRM )
        flagSignal=1;
}

int play(int* action, char** name){
    int m_id;
    int response = -1;
    msgToFate msgSend;
    fateResponse fateRes;

    *action = (rand() % 3)+1;

    if ((m_id = msgget(MSGIDFATE, 0666)) < 0)
        printf("**ERRORE** recupero coda di messaggi...\n");

    msgSend.mtype = IDFATE;
    msgSend.pid = getpid();
    msgSend.action = *action;

    if(flagSignal == 0){
        if (msgsnd(m_id, &msgSend, sizeof(msgSend), 0) < 0) {
            printf("[GIOCATORE_%s]**ERRORE** invio messaggio al Fato...\n", *name);
        } else {
            printf("[GIOCATORE_%s (%d)]Prende palla e prova l' AZIONE: %d\n",*name, getpid(), msgSend.action);
        }

        if ((msgrcv(m_id, &fateRes, sizeof(fateRes), getpid(), 0)) == -1)
            printf("[GIOCATORE_%s]**ERRORE** ricezione del messaggio da parte del giocatore\n", *name);
        else{
            response = fateRes.res;
            printf("[GIOCATORE_%s (%d)]Interroga il Fato che dice: = %d\n",*name,getpid(),response);
        }
    }
    return response;
}

