#include <errno.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <memory.h>
#include "header.h"



int volatile flagSignal = 0;
int main(int argc, char** arcgv){
    int m_id;
    int status;
    msgToFate msgRecived;
    fateResponse response;
    float actionProb;
    char* logMsg = malloc(sizeof(char)*200);
    char* statement;
    pid_t previousPid=0;
    int isSamePlayer=0;

    // VARIABILI LOG
    char *nomeDirectoryLog = malloc(sizeof(char)*50);

    printf("[FATO_%d] Processo creato\n",getpid());
    setupLogFile(&nomeDirectoryLog);

    if((m_id = msgget(MSGIDFATE, IPC_CREAT | 0666))<0)
        printf("**ERRORE** creazione coda di messaggi...\n");


    // SEGNALE SIGALARM CHE ARRIVA DA ARBITRO A FINE PARTITA
    sigAction.sa_handler = &sigHandler;
    sigAction.sa_flags = 0;
    sigaction (SIGALRM, NULL, &sigActionDef);
    if (sigActionDef.sa_handler != SIG_IGN)
        sigaction (SIGALRM, &sigAction, NULL);

    // CICLO DI ESECUZIONE, FINO A QUANDO NON VIENE RILEVATO UN SEGNALE DI FINE PARTITA
    while(flagSignal==0) {
        if ((msgrcv(m_id, &msgRecived, sizeof(msgRecived), IDFATE, 0)) == -1) {
            //FINE PARTITA
            if(flagSignal == 1){
                break;
            }
            printf("[FATO_%d]**ERRORE** ricezione del messaggio da parte del fato...\n", getpid());
        }
        else {
            actionProb = getProbability(msgRecived.action);
            printf("[FATO_%d]Messaggio ricevuto dal giocatore %d Azione %d con prob %f...\n", getpid(),msgRecived.pid,
                   msgRecived.action, actionProb);

            response.mtype = msgRecived.pid;
            response.res = askFate(actionProb);
            response.pid = getpid();
            sprintf(logMsg,"Giocatore %d compie azione:%d, fato dice:%d\n",msgRecived.pid,msgRecived.action,response.res);
            if(previousPid==msgRecived.pid)
                isSamePlayer = 1;
            else
                isSamePlayer = 0;
            statement = statementGenerator(msgRecived.action,response.res,isSamePlayer);

            sprintf(logMsg,"Giocatore %d compie azione:%d, %s\n",msgRecived.pid,msgRecived.action,statement);
            writeToLog(logMsg);

            if (msgsnd(m_id, &response, sizeof(response), 0) < 0) {
                printf("[FATO_%d]**ERRORE** invio messaggio al giocatore %d...\n", getpid(), msgRecived.pid);
            } else {
                printf("[FATO_%d]Risposta inviata al giocatore %d...\n", getpid(), msgRecived.pid);
                previousPid = msgRecived.pid;
            }

        }
    }
    //attende eventuale lettura dell'ultimo giocatore
    waitpid(msgRecived.pid, &status,0);
    printf("[FATO_%d]Fine dei responsi ai giocatori.\n", msgRecived.pid);
    free(nomeDirectoryLog);
    msgctl(m_id,IPC_RMID, (struct msqid_ds *)NULL);
    writeToLog("La partita è terminata, le squadre tornano negli spogliatoi");
    return 0;
}

void sigHandler(int sig) {
    if(sig == SIGALRM ){
        flagSignal=1;
    }
}

void setupLogFile(char **nomeDirectoryLog) {

    // CREAZIONE TEMP DIRECTORY
    sprintf(*nomeDirectoryLog,"../../log");
    if((mkdir(*nomeDirectoryLog, 0755))==-1){
        if(errno != EEXIST) {
            perror("[FATO]Errore directory root log");
            exit(EXIT_FAILURE);
        }
    }

    //CREAZIONE LOG DIRECTORY PER OGNI PROCESSO FATO
    sprintf(*nomeDirectoryLog,"../../log/LOG_%d",getpgid(0));
    if((mkdir(*nomeDirectoryLog, 0755))==-1){
        perror("[FATO]Errore directory root log");
        exit(EXIT_FAILURE);
    }

    writeToLog("Fato creato\n");

}

char* statementGenerator(int action, int response, int isSamePlayer){
    char *actionStatement = malloc(sizeof(char)*100);
    char *statement = malloc(sizeof(char)*100);

    if(isSamePlayer)
        strcpy(statement,"Tiene il pallone! ");
    else
        strcpy(statement,"Prende palla. ");

    switch (action) {
        case 1:
            if(response==0)
                strcpy(actionStatement,"Tenta il tiro, ma manca la porta!");
            else
                strcpy(actionStatement,"Tenta il tiro, ed è goal!!");
            break;
        case 2:
            if(response==0)
                strcpy(actionStatement,"Scontro in campo, il giocatore è illeso ma perde palla!");
            else
                strcpy(actionStatement,"Scontro in campo, il giocatore è infortunato!");
            break;
        case 3:
            if(response==0)
                strcpy(actionStatement,"Tenta il dribbling, ma fallisce e perde il pallone.");
            else {
                if (isSamePlayer == 0)
                    strcpy(actionStatement, "Tenta il dribbling, e riesce!");
                else
                    strcpy(actionStatement, "Incredibile, un altro dribbling!");
            }
            break;
        default:
            printf("[FATO_%d]**ERRORE**  ricevuta un azione non valida...\n", getpid());
    }
    strcat(statement,actionStatement);
    free(actionStatement);
    return statement;
}

void writeToLog(char *text){
    FILE *fp;
    char *nomeLogPath = malloc(sizeof(char)*50);

    sprintf(nomeLogPath,"../../log/LOG_%d/FATO_%d_LOG",getpgid(0),getpid());
    fp = fopen(nomeLogPath, "a");

    if(fprintf(fp,"%s\n",text)<=0){
        perror("[FATO]Errore scrittura log: ");
    }
    free(nomeLogPath);
    fclose(fp);
}

int askFate(double probability) {
    return rand() <  probability * ((double)RAND_MAX + 1.0);
}

float getProbability(int action) {
    float prob = 0;
    switch (action) {
        case 1:
            prob = getConfigValue("goalProb", CONFIGPATH)/100;
            break;
        case 2:
            prob = getConfigValue("injuryProb", CONFIGPATH)/100;
            break;
        case 3:
            prob = getConfigValue("dribblingProb", CONFIGPATH)/100;
            break;
        default:
            printf("[FATO_%d]**ERRORE**  ricevuta un azione non valida...\n", getpid());
    }
    return prob;
}



