#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include "header.h"

static Score matchScore;
int volatile flagSignal = 0;

int main(int argc, char **argcv) {

    matchScore.team0=0;
    matchScore.team1=0;
    int semID;
    int semIDTeam;
    pid_t pid;
    int status;
    int matchLength;
    char* teamA = getTeamSpecific("name", TEAM0);
	char* teamB = getTeamSpecific("name", TEAM1);


    /* SEGNALI: SIGALARM DI FINE PARTITA E SIGUSR DI NOTIFICA GOAL	*/
    sigAction.sa_handler = &sigHandler;
    sigAction.sa_flags = 0;
    sigaction (SIGALRM, NULL, &sigActionDef);
    if (sigActionDef.sa_handler != SIG_IGN) {
        sigaction(SIGALRM, &sigAction, NULL);
    }

    sigaction(SIGUSR1, &sigAction, NULL);
    sigaction(SIGUSR2, &sigAction, NULL);


    /*GENERAZIONE PALLONE */
    if((semID=semget(SEMID,1,IPC_CREAT | 0666)) ==-1)
        printf("[ARBITRO]**ERRORE** nella creazione del semaforo\n");

    if (initSemAvailable(semID,0) == -1)
        printf("[ARBITRO]**ERRORE** nell'inizializzazione del semaforo\n");


    /*GENERAZIONE FATO */
    createFato();

    /*GENERAZIONE SQUADRE */
    if((semIDTeam=semget(SEMIDTEAM,2,IPC_CREAT | 0666)) ==-1)
        printf("[ARBITRO]**ERRORE** nella creazione del semaforo\n");

    if (customInitSemAvailable(semIDTeam,0,5) == -1)
        printf("[ARBITRO]**ERRORE** nell'inizializzazione del semaforo del team 0\n");
    if (customInitSemAvailable(semIDTeam,1,5) == -1)
        printf("[ARBITRO]**ERRORE** nell'inizializzazione del semaforo del team 1\n");

    createSquadre();

    /*RECUPERO DURATA PARTITA DA FILE CONF */
    matchLength = (int) getConfigValue("matchLength", CONFIGPATH);
    alarm((unsigned int) matchLength);

    while(flagSignal==0){ pause();}

    /* OPERAZIONI DI FINE PARTITA */
    if(flagSignal==1) {

        /* RECUPERO PROCESSI FIGLI (2 SQUADRE E FATO) */
        while ((pid = waitpid(-1, &status, 0)) > 0) {
            printf("[ARBITRO] Processo figlio raccolto: %d\n",pid);
            fflush(stdout);
        }

        /* RIMOZIONE SEMAFORI */
        semctl(semID, 0, IPC_RMID);
        semctl(semIDTeam, 0, IPC_RMID);
    }

    printf("[ARBITRO] Il risultato finale della partita %s vs %s è: %d - %d\n",teamA,teamB,matchScore.team0,matchScore.team1);
    return 0;
}

void sigHandler(int sig) {
    if(sig == SIGALRM && (sigActionDef.sa_handler != SIG_IGN)){
        pid_t  pgid = getpgid(0);
        sigActionDef.sa_handler = SIG_IGN; /* dato che mi appresto a inviare signal a tutto il gruppo (quindi arbitro incluso), aggiungo ignore */
        sigaction (SIGALRM, &sigAction, NULL);
        printf("\n\n[ARBITRO] FINE PARTITA.\n\n\n");

        if(kill(-pgid,SIGALRM) >0){
            printf("[ARBITRO]**ERRORE** kill >0\n");
        }
        flagSignal=1;
    }
    if(sig == SIGUSR1) {
        matchScore.team0=matchScore.team0+1;
        printf("[ARBITRO] È stato seganto un gol dalla squadra A. Punteggio aggiornato %d - %d\n",matchScore.team0, matchScore.team1);

    }
    if(sig==SIGUSR2){
        matchScore.team1=matchScore.team1+1;
        printf("[ARBITRO] È stato seganto un gol dalla squadra B. Punteggio aggiornato: %d - %d\n",matchScore.team0, matchScore.team1);

    }
}

void createFato() {
    fflush(stdout);
    pid_t i = fork();
    if(i == 0){
        execve("./Fato",(char *[]){ "./Fato", NULL, NULL }, NULL);
        _exit(EXIT_FAILURE);
    }
    else if (i > 0){
        printf("[ARBITRO_%d] - Genero processo fato con pid %d\n",getpid(),i);
        return;
    }
    else{
        perror("[ARBITRO_%d]Fork fallito\n");
        exit(EXIT_FAILURE);
    }
}

void createSquadre() {
    int c=0;
    char teamLetter = 65; //65 corrisponde alla lettera A
    char* numTeam;
    numTeam =(char*) malloc(sizeof(char));

    while(c<2) {
        *numTeam = (char) c;
        char *argv[] = { numTeam, NULL };
        char *envp[] = { NULL };

        pid_t i = fork();
        if (i == 0) {
            execve("./Squadra",argv, envp);
            _exit(EXIT_FAILURE);
        } else if (i > 0) {
            printf("[ARBITRO_%d] - Genero processo squadra %c con pid %d\n",getpid(),teamLetter, i);
            teamLetter++;
            c++;
        } else {
            perror("[ARBITRO]Fork fallito\n");
            exit(EXIT_FAILURE);
        }
    }
    free(numTeam);
}



