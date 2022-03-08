/** @file
  Implementacja funkcji potrzebnych do wykonania instrukcji.

  @author Mikołaj Szkaradek
  @date 2021
*/
#include "executing_instruction.h"
#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>

#define ENDL '\n'               ///< Stała oznaczająca znak '\n'.

#define ASCII_ZERO '0'          ///< Stała oznaczająca znak '0'.
#define ASCII_NINE '9'          ///< Stała oznaczająca znak '9'.
#define DECIMAL_BASE 10         ///< Stała oznaczająca bazę systemu dziesiątkowego.
#define SPACE ' '               ///< Stała oznaczająca znak ' '.
#define MINUS '-'               ///< Stała oznaczająca znak '#'.

// Instrukcje.
#define ZERO "ZERO"             ///< Stała oznaczająca polecenie ZERO.
#define IS_COEFF "IS_COEFF"     ///< Stała oznaczająca polecenie IS_COEFF.
#define IS_ZERO "IS_ZERO"       ///< Stała oznaczająca polecenie IS_ZERO.
#define CLONE "CLONE"           ///< Stała oznaczająca polecenie CLONE.
#define ADD "ADD"               ///< Stała oznaczająca polecenie ADD.
#define MUL "MUL"               ///< Stała oznaczająca polecenie MUL.
#define NEG "NEG"               ///< Stała oznaczająca polecenie NEG.
#define SUB "SUB"               ///< Stała oznaczająca polecenie SUB.
#define IS_EQ "IS_EQ"           ///< Stała oznaczająca polecenie IS_EQ.
#define DEG "DEG"               ///< Stała oznaczająca polecenie DEG.
#define DEG_BY "DEG_BY"         ///< Stała oznaczająca polecenie DEG_BY.
#define AT "AT"                 ///< Stała oznaczająca polecenie AT.
#define PRINT "PRINT"           ///< Stała oznaczająca polecenie PRINT.
#define POP "POP"               ///< Stała oznaczająca polecenie POP.
#define COMPOSE "COMPOSE"       ///< Stała oznaczająca polecenie COMPOSE.

/**
 * Stała oznaczająca index, na którym w poleceniu AT powinna wystąpić
 * cyfra lub '-'.
 */
#define AT_DIGIT_IDX 3
/**
 * Stała oznaczająca index, na którym w poleceniu DEG_BY powinna wystąpić
 * cyfra.
 */
#define DEG_BY_DIGIT_IDX 7
/**
 * Stała oznaczająca index, na którym w poleceniu COMPOSE powinna wystąpić
 * cyfra.
 */
#define COMPOSE_DIGIT_IDX 8

// Identyfikatory instrukcji.
#define ADD_ID 'A'              ///< Stała na identyfikator instrukcji ADD.
#define SUB_ID 'S'              ///< Stała na identyfikator instrukcji SUB.
#define MUL_ID 'M'              ///< Stała na identyfikator instrukcji MUL.

/**
 * Funkcja sprawdza, czy znak jest cyfrą.
 */
static bool IsDigit(char c) {
    if (c >= ASCII_ZERO && c <= ASCII_NINE) return true;
    else return false;
}

/**
 * Funkcja sprawdza, czy znak jest cyfrą lub znakiem '-'.
 */
static bool IsDigitOrMinus(char c) {
    return (IsDigit(c) || c == MINUS);
}

/**
 * Funkcja próbuje wywołać funkcję DegBy. Sprawdza, czy parametr idx jest poprawny.
 * Jeżeli tak, wywołuje funkcję, jeżeli nie to wypisuje na
 * standardowe wyjście diagnostyczne: ERROR w DEG BY WRONG VARIABLE\n.
 */
static void AttemptDegBy(Stack **Polynomials, const char *line, int line_number) {
    if (!IsDigit(line[DEG_BY_DIGIT_IDX]) || line[DEG_BY_DIGIT_IDX - 1] != SPACE) {
        fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
    }
    else {
        line += DEG_BY_DIGIT_IDX;
        char *end;
        errno = 0;
        unsigned long idx = strtoul(line, &end, DECIMAL_BASE);
        if (*end != 0) {
            fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
        }
        else {
            if (idx == ULONG_MAX && errno == ERANGE) {
                fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
            }
            else {
                DegBy(Polynomials, idx, line_number);
            }
        }
    }
}

/**
 * Funkcja próbuje wywołać funkcję DegBy. Sprawdza, czy parametr x jest poprawny.
 * Jeżeli tak, wywołuje funkcję, jeżeli nie to wypisuje na
 * standardowe wyjście diagnostyczne: ERROR w AT WRONG VARIABLE\n.
 */
static void AttemptAt(Stack **Polynomials, const char *line, int line_number) {
    if (!IsDigitOrMinus(line[AT_DIGIT_IDX]) || line[AT_DIGIT_IDX - 1] != SPACE) {
        fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
    }
    else {
        line += AT_DIGIT_IDX;
        char *end;
        errno = 0;
        poly_coeff_t x = strtol(line, &end, DECIMAL_BASE);
        if (*end != 0) {
            fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
        }
        else {
            if ((x == LONG_MAX || x == LONG_MIN) && errno == ERANGE) {
                fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
            }
            else {
                At(Polynomials, x, line_number);
            }
        }
    }
}

/**
 * Funkcja próbuje wywołać funkcję COMPOSE. Sprawdza, czy parametr count jest poprawny.
 * Jeżeli tak, wywołuje funkcję, jeżeli nie to wypisuje na
 * standardowe wyjście diagnostyczne: ERROR w COMPOSE WRONG PARAMETER\n.
 */
static void AttemptCompose(Stack **Polynomials, const char *line, int line_number) {
    if (!IsDigit(line[COMPOSE_DIGIT_IDX]) || line[COMPOSE_DIGIT_IDX - 1] != SPACE) {
        fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
    }
    else {
        line += COMPOSE_DIGIT_IDX;
        char *end;
        errno = 0;
        size_t count = strtoul(line, &end, DECIMAL_BASE);
        if (*end != 0) {
            fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
        }
        else {
            if (((count == ULONG_MAX) || count == 0 ) && errno == ERANGE) {
                fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);;
            }
            else {
                Compose(Polynomials, count, line_number);
            }
        }
    }
}

void ExecuteInstruction(Stack **Polynomials, char *instruction,
                        const char *const_line, int line_number, size_t line_size) {
    char *line = malloc((line_size + 1) * sizeof(char));
    if (line == NULL) exit(1);
    strcpy(line, const_line);
    if (line[line_size - 1] == ENDL) {
        line[line_size - 1] = 0;
    }
    else {
        line_size++;
    }
    if (strlen(instruction) == line_size - 1) {
        if (strcmp(instruction, ZERO) == 0) Zero(Polynomials);
        else if (strcmp(instruction, IS_COEFF) == 0) IsCoeff(Polynomials, line_number);
        else if (strcmp(instruction, IS_ZERO) == 0) IsZero(Polynomials, line_number);
        else if (strcmp(instruction, CLONE) == 0) Clone(Polynomials, line_number);
        else if (strcmp(instruction, ADD) == 0) AddSubOrMul(Polynomials, line_number, ADD_ID);
        else if (strcmp(instruction, MUL) == 0) AddSubOrMul(Polynomials, line_number, MUL_ID);
        else if (strcmp(instruction, SUB) == 0) AddSubOrMul(Polynomials, line_number, SUB_ID);
        else if (strcmp(instruction, NEG) == 0) Neg(Polynomials, line_number);
        else if (strcmp(instruction, IS_EQ) == 0) IsEq(Polynomials, line_number);
        else if (strcmp(instruction, DEG) == 0) Deg(Polynomials, line_number);
        else if (strcmp(instruction, PRINT) == 0) Print(Polynomials, line_number);
        else if (strcmp(instruction, POP) == 0) PopPoly(Polynomials, line_number);
        else if (strcmp(instruction, DEG_BY) == 0) fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n",
                                                           line_number);
        else if (strcmp(instruction, AT) == 0) fprintf(stderr, "ERROR %d AT WRONG VALUE\n",
                                                       line_number);
        else if (strcmp(instruction, COMPOSE) == 0) fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n",
                                                            line_number);
        else fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
        free(line);
    }
    else {
        if (strcmp(instruction, DEG_BY) == 0) {
            AttemptDegBy(Polynomials, line, line_number);
        }
        else if (strcmp(instruction, AT) == 0) {
            AttemptAt(Polynomials, line, line_number);
        }
        else if (strcmp(instruction, COMPOSE) == 0) {
            AttemptCompose(Polynomials, line, line_number);
        }
        else {
            fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
        }
        free(line);
    }
}
