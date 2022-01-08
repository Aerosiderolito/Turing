#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include <stdbool.h>

typedef unsigned char byte;
typedef int error_code;

#define ERROR (-1)
#define HAS_ERROR(code) ((code) < 0)
#define HAS_NO_ERROR(code) ((code) >= 0)

/**
 * Cette fonction compare deux chaînes de caractères.
 * @param p1 la première chaîne
 * @param p2 la deuxième chaîne
 * @return le résultat de la comparaison entre p1 et p2. Un nombre plus petit que
 * 0 dénote que la première chaîne est lexicographiquement inférieure à la deuxième.
 * Une valeur nulle indique que les deux chaînes sont égales tandis qu'une valeur positive
 * indique que la première chaîne est lexicographiquement supérieure à la deuxième.
 */
int strcmp(char *p1, char *p2) {
    char *s1 = (char *) p1;
    char *s2 = (char *) p2;
    char c1, c2;
    do {
        c1 = (char) *s1++;
        c2 = (char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}

/**
 * Ex. 1: Calcul la longueur de la chaîne passée en paramètre selon
 * la spécification de la fonction strlen standard
 * @param s un pointeur vers le premier caractère de la chaîne
 * @return le nombre de caractères dans le code d'erreur, ou une erreur
 * si l'entrée est incorrecte
 */
error_code strlen2(char *s) {

    int index = 0;
    int length = 0;

    //on parcour la string à l'aide d'un pointeur et on incrémente
    //length de 1 à chaque fois. Lorsqu'on arrive à '\0' on arrête
    // et on retourne la nbBytes qui contient la longueur de la string.
    if (s != NULL) {
        while(1){
            if (s[index] == '\0') {
                if (index == 0) {
                    error_code nbByte = 0;
                    return nbByte;
                }
                error_code nbByte = length;
                return nbByte;
            }else{
                length++;
            }

            index++;

        }
    } else {
        return ERROR;
    }
}

/**
 * Ex.2 :Retourne le nombre de lignes d'un fichier sans changer la position
 * courante dans le fichier.
 * @param fp un pointeur vers le descripteur de fichier
 * @return le nombre de lignes, ou -1 si une erreur s'est produite
 */
error_code no_of_lines(FILE *fp) {

    if(fp == NULL){
        perror("Error: ");
        return ERROR;
    }

    int offset = 0;
    int lineCount = 0;


    while(1){
        //ici on utilise un offset qu'on incrémente à chaque fois.
        //On reset le pointeur à la position 0 et ensuite on change la position
        // à l'aide de l'offset.
        fseek(fp,0, SEEK_SET);
        fseek(fp,offset, SEEK_CUR);
        int currentCharInt = getc(fp); //on va chercher le character à la position
        //actuelle.


        // '\n' pour détecter une nouvelle ligne.
        if (currentCharInt == '\n') {
            lineCount++;
        }

        //fin du fichier.
        if (currentCharInt == EOF) {

            //détecter le cas spécial ou on a un '\n' suivi de EOF.
            fseek(fp,0,SEEK_SET);
            //on va chercher le caractère précédent à EOF.
            fseek(fp,offset-1,SEEK_CUR);
            //si le caractère précédent n'est pas un '\n'
            //alors on incrémente.
            if (getc(fp) != '\n'){
                lineCount++;
            }


            //le cas où le fichier est vide
            //on set le lineCount à 0
            if (offset == 0){
                lineCount = 0;
            }

            //On reset le pointeur pour qu'il pointe au premier caractère
            fseek(fp,1, SEEK_SET);
            error_code nbOfLines = lineCount;

            return nbOfLines;
        }

        offset++;
    }


}

/**
 * Ex.3: Lit une ligne au complet d'un fichier
 * @param fp le pointeur vers la ligne de fichier
 * @param out le pointeur vers la sortie
 * @param max_len la longueur maximale de la ligne à lire
 * @return le nombre de caractère ou ERROR si une erreur est survenue
 */
error_code readline(FILE *fp, char **out, size_t max_len) {

    // Declaration des variables
    out[0] = malloc(sizeof(char)*max_len);
    int nvChar = getc(fp);
    int nbChar = 0;
    bool boucle = true;

    // Verifie qu'il reste des caracteres
    if(nvChar == EOF){
        boucle = false;
    }

    // Ajout des char a la liste de char (le pointeur "out")
    while(boucle){

        // Ajout du char a la liste si dans la meme ligne
        if(nvChar != '\r' & nvChar != '\n'){
            out[0][nbChar] = (char)nvChar;
            nbChar++;
        }

        // Fetch nouveau char
        nvChar = getc(fp);

        // S'assure nombre maximum de char n'est pas atteint OU fin du fichier
        if(nvChar == EOF | nbChar == max_len-1){
            boucle = false;
        }

        // S'assure de commencer au debut d'une ligne
        if(nvChar == '\r' | nvChar == '\n'){
            if(nbChar>0){
                boucle = false;
            }
        }

    }

    // Ajout d'une fin de ligne
    out[0][nbChar] = '\0';
    nbChar++;

    // Retourne le nombre de char dans la ligne
    return nbChar;
}

/**
 * Ex.4: Copie un bloc mémoire vers un autre
 * @param dest la destination de la copie
 * @param src  la source de la copie
 * @param len la longueur (en byte) de la source
 * @return nombre de bytes copiés ou une erreur s'il y a lieu
 */
error_code memcpy2(void *dest, void *src, size_t len) {

    // Cast des espaces memoires en byte
    byte *destCast = (byte*)dest;
    byte *srcCast = (byte*)src;

    // Copy des bytes 1 par 1
    for (int i = 0; i < len; i++){
        destCast[i] = srcCast[i];
    }

    // Retourne le nombre de byte copie
    return len;
}

/**
* Ex.5: Analyse une ligne de transition
* @param line la ligne à lire
* @param len la longueur de la ligne
* @return la transition ou NULL en cas d'erreur
*/
transition *parse_line(char *line, size_t len) {

    transition *retValue = malloc( len*sizeof( transition ));

    char movement, read, write;

    char *etatInitiale = malloc(len*sizeof (char*));
    char *nextState = malloc(len*sizeof (char*));
    char separateur = ',';

    // Loop pour l'etat initiale
    int counter=0; // pour compter le nombre de virgules
    bool testEI = false;
    bool readTest = false;
    bool nxState = false;

    // pour prendre l'etat initiale
    for(int i =0; line[i] != separateur; i++){

        if(line[i]=='('){
            continue;
        }
        else{
            etatInitiale[i-1] = line[i];
        }
    }

    // pour prendre le character lu
    for(int i =0; line[i] != ')'; i++){
        if(line[i]=='('){
            continue;
        }
        else{
            if (line[i]==separateur){
                i++;
                read = line[i];
                readTest = true;
                break;
            }
        }
    }

    // pour prendre l'etat suivant
    for(int i =0; counter<2 ; i++){
        if(line[i]=='(' || line[i]==')' || line[i]=='-' || line[i]=='>' || line[i]==read){
            continue;
        }
        else{
            if (line[i]==separateur){
                counter++;
                continue;

            }
            if (counter==1 ){
                for(int j =0; j<len;j++){
                    nextState[j] = line[i];
                    i++;
                    if (line[i]==separateur){
                        counter = 2;
                        break;
                    }
                }
            }
        }
    }

    counter = 0;

    // prendre le char a ecrire
    for(int i = 0; counter<3; i++){

        if(line[i]==separateur){
            counter++;
            continue;
        }
        if (counter==2){
            write = line[i];
            counter = 3;
        }
    }

    counter = 0;
    // pour trouver le mouvement
    for (int i = 0;  counter<4 ; ++i) {

        if(line[i]==separateur){
            counter++;
            continue;
        }
        if (counter==3){

            if (line[i]=='G'){
                movement = -1;
            }
            if(line[i] == 'R'){
                movement = 0;
            }
            if(line[i] == 'D'){
                movement = 1;
            }
            counter = 4;
        }
    }

    retValue->next_state = (char* )nextState;
    retValue->movement = movement;
    retValue->write = write;
    retValue->read = read;
    retValue->current_state = (char* )etatInitiale;

    if (retValue->read == NULL){
        return ERROR;
    }
    return retValue;
}

/**
 * Ex.6: Execute la machine de turing dont la description est fournie
 * @param machine_file le fichier de la description
 * @param input la chaîne d'entrée de la machine de turing
 * @return le code d'erreur
 */
error_code execute(char *machine_file, char *input) {

    // Si le file existe pas on retourne -1
    if (fopen(machine_file, "r") == NULL){
        return ERROR;
    }

    FILE *fp = fopen(machine_file, "r");
    char **read = malloc(sizeof(char*));
    int nbLines = no_of_lines(fp);
    int longeur, counter = 0;
    transition *tabTransitions = (transition*)malloc(nbLines*sizeof(transition));

    // Construit un tableau contenant les transitions
    for(int i = 0 ; i < nbLines; i++){
        longeur = readline(fp,read,nbLines);
        if(i>2) {
            tabTransitions[counter] = *parse_line(*read, longeur);
            counter++;
        }
    }

    //Allocation du ruban
    char ruban[strlen2(input)*2];

    //ajoute input sur le ruban
    for(int i=0; i < strlen2(input); i++){
        ruban[i]=input[i];
    }

    //ajoute des " " pour le reste d'index dans ruban
    for(int i=strlen2(input); i < sizeof(ruban); i++){
        ruban[i]=' ';
    }

    //copier les mots sur le ruban de travail
    memcpy2(ruban,input,strlen2(input));

    //stock currenstate pour utilise dans for loop
    char *currentState =tabTransitions[0].current_state;
    char head;
    bool boolT;
    //executer la machine jusqau la fin de ruban
    //sauf si on trouve R ou A dans le for loop imbrique
    for(int i = 0; i < sizeof(ruban); i++){

        if ( ruban[i] != '0' && ruban[i] != '1' && ruban[i] != ' '){
            return ERROR;
        }
        head = ruban[i];
        for(int j = 0; j < sizeof(nbLines); j++){

            if( head == tabTransitions[j].read){

                if(strcmp("A",tabTransitions[j].next_state) == 0){
                    return 1;
                }
                if(strcmp("R",tabTransitions[j].next_state) == 0){
                    return 0;
                }
            }
            currentState = tabTransitions[j].next_state;
            break;
        }
    }
    fclose(fp);
    free(*read);

}


// ATTENTION! TOUT CE QUI EST ENTRE LES BALISES ༽つ۞﹏۞༼つ SERA ENLEVÉ! N'AJOUTEZ PAS D'AUTRES ༽つ۞﹏۞༼つ

// ༽つ۞﹏۞༼つ

int main() {




}

// ༽つ۞﹏۞༼つ