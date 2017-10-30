#include "header.h"

int main(int argc, char **argcv) {
    showMenu();
}

void showMenu(){
    char input;
    system("clear");
    printf("------ Benvenuto su CalcettoOS! ------\n\n"
                   "-Premi a per avviare la partita Juventus FC vs Barcellona FC\n"
                   "-Premi s per settare la durata della partita\n"
                   "-Premi ? per visualizzare guida & credits\n"
                   "-Premi q per uscire\n\n");
    printf("Scelta: ");
    scanf(" %c", &input);

    switch(input){
        case 'a':
            execve("./Arbitro",(char *[]){ "./Arbitro", NULL, NULL }, NULL);
            _exit(EXIT_FAILURE);
        case '?':
            showGuide();
            break;
        case 's':
            editConf();
            break;
        case 'q':
            exit(0);
        default:break;
    }

}

void editConf() {
    FILE *pFile;
    pFile = fopen(CONFIGPATH, "r+");
    char* command = malloc(sizeof(char)*50);
    int input;

    system("clear");
    printf("Il valore deve essere compreso tra 1 e 99, eventuali valori decimali non verranno considerati.\nInserisci valore:");
    scanf("%d", &input);
    if(input && input>0 && input<100) {
        sprintf(command, "sed -i 1s/.*/'matchLength %d'/ %s", input, CONFIGPATH);
        system(command);
    }else{
        printf("Errore, la durata deve essere un numero compreso tra 1 e 99");
    }
    free(command);
    fclose(pFile);
    showMenu();
}


void showGuide() {
    char input;
    system("clear");
    system("cat ../../docs/project.md" );
    printf("\n\nPremi un tasto e invio per tornare alla schermata iniziale\n");
    scanf(" %c",&input);
    showMenu();
}