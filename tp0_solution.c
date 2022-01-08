#include <stdlib.h>
#include <stdio.h>

typedef unsigned char byte;
typedef int error_code;

#define ERROR (-1)
#define HAS_ERROR(code) ((code) < 0)
#define HAS_NO_ERROR(code) ((code) >= 0)
#define MAX_LINE_LENGTH (22)

/**
 * Calcul la longueur de la chaîne passée en paramètre selon
 * la spécification de la fonction strlen standard
 * @param s un pointeur vers le premier caractère de la chaîne
 * @return le nombre de caractères
 */
error_code strlen(char *s) {
    if (NULL == s) { return ERROR; }
    char *p = s;
    while (*p != '\0') {
        p++;
    }
    return p - s;
}

/**
 * Retourne le nombre de lignes d'un fichier sans changer la position
 * courante dans le fichier.
 * @param fp un pointeur vers le descripteur de fichier
 * @return le nombre de lignes, ou -1 si une erreur s'est produite
 */
error_code no_of_lines(FILE *fp) {
    if (NULL == fp) { return ERROR; }

    size_t lc = 0;
    long position = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int c = 0;
    unsigned char last_was_nl = 1;
    while (EOF != (c = getc(fp))) {
        lc += (last_was_nl = (c == '\n'));
    }

    lc += (EOF == c && !last_was_nl);

    fseek(fp, position, SEEK_SET);
    return lc;
}

/**
 * Lit une ligne au complet d'un fichier
 * @param fp le pointeur vers la ligne de fichier
 * @param out le pointeur vers la sortie
 * @param max_len la longueur maximale de la ligne à lire
 * @return le nombre de caractères lu ou ERROR si une erreur est survenue
 */
error_code readline(FILE *fp, char **out, size_t max_len) {
    if (NULL == fp) return ERROR;
    if (NULL == out) return ERROR;

    char *allocated = (char *) malloc(max_len + 1);

    if (NULL == allocated) { return ERROR; }

    size_t cc = 0;
    for (; cc < max_len; ++cc) {
        int chr = getc(fp);
        if (EOF == chr || '\n' == chr) {
            break;
        } else {
            allocated[cc] = (char) chr;
        }
    }
    allocated[cc] = '\0';
    *out = allocated;
    return cc;
}

/**
 * Structure qui dénote une transition de la machine de Turing
 */
typedef struct {
    char *current_state;
    char *next_state;
    char movement;
    char read;
    char write;
} transition;

/**
 * Copie un bloc mémoire vers un autre
 * @param dest la destination de la copie
 * @param src  la source de la copie
 * @param len la longueur (en byte) de la source
 * @return nombre de bytes copiés ou une erreur
 */
error_code memcpy(void *dest, void *src, size_t len) {
    if (NULL == dest) return ERROR;
    if (NULL == src) return ERROR;

    byte *d = (byte *) dest;
    byte *s = (byte *) src;

    for (size_t i = 0; i < len; ++i) {
        d[i] = s[i];
    }

    return len;
}

int skip_to(byte stop_on_special, char *l, int index) {
    while (('\0' != l[index]) && stop_on_special == ('(' == l[index] ||
                                                     ')' == l[index] ||
                                                     '-' == l[index] ||
                                                     '>' == l[index] ||
                                                     ',' == l[index])) {
        index++;
    }
    return index;
}

void clean(byte *b, size_t len) {
    for (int i = 0; i < len; ++i) {
        b[i] = 0;
    }
}

/**
 * Analyse une ligne de transition
 * @param line la ligne à lire
 * @param len la longueur de la ligne
 * @return la transition ou NULL en cas d'erreur
 */
transition *parse_line(char *line, size_t len) {
    if (NULL == line || 0 == len) { return NULL; }
    byte error = 0;

    int s = skip_to(1, line, 0);
    int e = skip_to(0, line, s);

    transition *t = NULL;
    char *start_state = NULL;
    char *end_state = NULL;

    if (NULL == (t = malloc(sizeof(transition)))) {
        error = 1;
        goto parse_line_cleanup;
    } else if (NULL == (start_state = malloc(sizeof(char) * (e - s + 1)))) {
        error = 1;
        goto parse_line_cleanup;
    }

    memcpy(start_state, line + s, e - s);
    start_state[e - s] = '\0';

    char read = line[++e];

    s = skip_to(1, line, e + 1);
    e = skip_to(0, line, s);

    if (NULL == (end_state = malloc(sizeof(char) * (e - s + 1)))) {
        error = 1;
        goto parse_line_cleanup;
    }

    memcpy(end_state, line + s, e - s);
    end_state[e - s] = '\0';

    char write = line[++e];
    char dir_chr = line[e + 2];

    byte dir = 0;

    if (dir_chr == 'G') {
        dir -= 1;
    } else if (dir_chr == 'D') {
        dir += 1;
    }

    t->current_state = start_state;
    t->next_state = end_state;
    t->read = read;
    t->write = write;
    t->movement = dir;

    parse_line_cleanup:

    if (error) {

        if (NULL != t) { free(t); }

        if (NULL != start_state) {
            free(start_state);
        }

        if (NULL != end_state) {
            free(end_state);
        }
    }

    return t;
}

/**
 * GNU STRCMP
 * @param p1
 * @param p2
 * @return
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

void clean_tape(char *tape, int tape_len) {
    for (int i = 0; i < tape_len; ++i) {
        tape[i] = ' ';
    }
}

/**
 * Execute la machine de turing dont la description est fournie
 * @param machine_file le fichier de la description
 * @param input la chaîne d'entrée de la machine de turing
 * @return le code d'erreur
 */
error_code execute(char *machine_file, char *input) {
    if (NULL == machine_file) return ERROR;
    if (NULL == input) return ERROR;

    FILE *fp = fopen(machine_file, "r");

    if (NULL == fp) {
        return ERROR;
    }

    error_code err = 0;
    size_t line_count = 0;
    char *tape = NULL;

    char *init_state = NULL;
    char *accept_state = NULL;
    char *reject_state = NULL;

    if (HAS_ERROR(readline(fp, &init_state, 5))) {
        err = ERROR;
        goto execute_cleanup;
    }

    if (HAS_ERROR(readline(fp, &accept_state, 5))) {
        err = ERROR;
        goto execute_cleanup;
    }

    if (HAS_ERROR(readline(fp, &reject_state, 5))) {
        goto execute_cleanup;
    }

    if (HAS_ERROR((line_count = no_of_lines(fp)))) {
        err = ERROR;
        goto execute_cleanup;
    }

    transition **states = (transition **) malloc(sizeof(transition *) * (line_count - 3));

    // init the buffer
    for (int i = 0; i < (line_count - 3); ++i) {
        states[i] = NULL;
    }

    // fill with real transitions
    for (int i = 0; i < (line_count - 3); ++i) {
        char *line = NULL;
        error_code linerr = 0;

        if (HAS_NO_ERROR(linerr = readline(fp, &line, MAX_LINE_LENGTH))) {
            states[i] = parse_line(line, linerr);
            free(line);
            if (NULL == states[i]) {
                err = ERROR;
                goto execute_cleanup;
            }
        } else {
            err = ERROR;
            goto execute_cleanup;
        }
    }

    char *current_state = init_state;

    size_t ipt_len = strlen(input);
    size_t tape_len = (2 * (ipt_len == 0 ? 4 : ipt_len));
    int tape_head = 0;
    tape = malloc(sizeof(char) * (tape_len + 1));

    if (NULL == tape) {
        err = ERROR;
        goto execute_cleanup;
    } else {
        clean_tape(tape, tape_len);
        memcpy(tape, input, ipt_len);
    }

    while (HAS_NO_ERROR(err) && ((err >> 1U) == 0)) {
        // Look for transition
        byte transitioned = 0;
        for (int i = 0; i < line_count - 3; ++i) {
            transition *t = states[i];
            if (0 == strcmp(t->current_state, current_state) && tape[tape_head] == t->read) {
#ifdef PRINT_MT_LOG
                printf("Before: %s\n", tape);
#endif
                transitioned = 1;
                current_state = t->next_state;
                tape[tape_head] = t->write;
#ifdef PRINT_MT_LOG
                printf("After:  %s\n", tape);
#endif
                tape_head += t->movement;
#ifdef PRINT_MT_LOG
                printf("----------------\n");
#endif
                break;
            }
        }

        if (!transitioned) {
            err = ERROR;
        } else if (tape_head < 0) {
            // Block going negative
            tape_head = 0;
        } else if (tape_head >= tape_len) {
            char *new_tape = malloc(sizeof(char) * (2 * tape_len));
            if (NULL == new_tape) { err = ERROR; }
            else {
                // Grow the tape
                memcpy(new_tape, tape, tape_len);
                clean_tape(new_tape + tape_len, tape_len);
                tape_len *= 2;
                free(tape);
                tape = new_tape;
            }
        }

        if (0 == strcmp(current_state, accept_state)) {
            err |= 3;
        } else if (0 == strcmp(current_state, reject_state)) {
            err |= 2;
        }
    }

    execute_cleanup:
    if (NULL != fp) {
        fclose(fp);
    }

    if (NULL != init_state) free(init_state);
    if (NULL != accept_state) free(accept_state);
    if (NULL != reject_state) free(reject_state);

    if (NULL != states && line_count > 3) {
        for (int i = 0; i < line_count - 3; ++i) {
            if (NULL != states[i]) {
                transition *t = states[i];
                free(t->current_state);
                free(t->next_state);
                free(t);
            }
        }
        free(states);
    }

    if (NULL != tape) {
        free(tape);
    }

    return HAS_ERROR(err) ? err : (err & 1U);
}