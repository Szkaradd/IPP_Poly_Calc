/** @file
  Implementacja kalkulatora wielomianów rzadkich wielu zmiennych.

  @author Mikołaj Szkaradek
  @date 2021
*/
#define _GNU_SOURCE     ///< GNU_SOURCE.

#include "poly.h"
#include "stack.h"
#include "instructions.h"
#include "executing_instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/// Makro funkcji PolyFromCoeff.
#define C PolyFromCoeff

// Znaki.
#define COMMENT '#'             ///< Stała oznaczająca znak '#'.
#define OPEN_PARENTHESIS '('    ///< Stała oznaczająca znak '('.
#define CLOSE_PARENTHESIS ')'   ///< Stała oznaczająca znak ')'.
#define PLUS '+'                ///< Stała oznaczająca znak '+'.
#define COMMA ','               ///< Stała oznaczająca znak ','.
#define MINUS '-'               ///< Stała oznaczająca znak '#'.
#define ENDL '\n'               ///< Stała oznaczająca znak '\n'.
#define ASCII_ZERO '0'          ///< Stała oznaczająca znak '0'.
#define ASCII_NINE '9'          ///< Stała oznaczająca znak '9'.
#define AT_FIRST_CHAR 'A'       ///< Stała oznaczająca pierwszy znak polecenia AT.
#define DEG_BY_FIRST_CHAR 'D'   ///< Stała oznaczająca pierwszy znak polecenia DEG_BY.
#define COMPOSE_FIRST_CHAR 'C'  ///< Stała oznaczająca pierwszy znak polecenia COMPOSE.

// Stałe liczbowe.
#define DECIMAL_BASE 10         ///< Stała oznaczająca bazę systemu dziesiątkowego.
#define INITIAL_SIZE 4          ///< Stała oznaczająca początkowy rozmiar tablicy.
#define IDX_1 1                 ///< Stała oznaczająca index nr 1 w tablicy.
#define IDX_2 2                 ///< Stała oznaczająca index nr 2 w tablicy.
#define IDX_3 3                 ///< Stała oznaczająca index nr 3 w tablicy.
#define IDX_4 4                 ///< Stała oznaczająca index nr 4 w tablicy.
#define IDX_5 5                 ///< Stała oznaczająca index nr 5 w tablicy.
#define IDX_6 6                 ///< Stała oznaczająca index nr 6 w tablicy.

#define AT_SECOND_CHAR 'T'      ///< Stała oznaczająca drugi znak polecenia AT.

#define DEG_BY_SECOND_CHAR 'E'  ///< Stała oznaczająca drugi znak polecenia DEG_BY.
#define DEG_BY_THIRD_CHAR 'G'   ///< Stała oznaczająca trzeci znak polecenia DEG_BY.
#define DEG_BY_FOURTH_CHAR '_'  ///< Stała oznaczająca czwarty znak polecenia DEG_BY.
#define DEG_BY_FIFTH_CHAR 'B'   ///< Stała oznaczająca piąty znak polecenia DEG_BY.
#define DEG_BY_SIXTH_CHAR 'Y'   ///< Stała oznaczająca szósty znak polecenia DEG_BY.

#define COMPOSE_SECOND_CHAR 'O'  ///< Stała oznaczająca drugi znak polecenia COMPOSE.
#define COMPOSE_THIRD_CHAR 'M'   ///< Stała oznaczająca trzeci znak polecenia COMPOSE.
#define COMPOSE_FOURTH_CHAR 'P'  ///< Stała oznaczająca czwarty znak polecenia COMPOSE.
#define COMPOSE_FIFTH_CHAR 'O'   ///< Stała oznaczająca piąty znak polecenia COMPOSE.
#define COMPOSE_SIXTH_CHAR 'S'   ///< Stała oznaczająca szósty znak polecenia COMPOSE.
#define COMPOSE_SEVENTH_CHAR 'E' ///< Stała oznaczająca siódmy znak polecenia COMPOSE.

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

// Litery.
#define CAPITAL_A 'A'           ///< Stała oznaczająca literę A.
#define SMALL_A 'a'             ///< Stała oznaczająca literę a.
#define CAPITAL_Z 'Z'           ///< Stała oznaczająca literę Z.
#define SMALL_Z 'z'             ///< Stała oznaczająca literę z.

void PolyPrint(const Poly *p) {
    if (PolyIsCoeff(p)) {
        printf("%ld", p->coeff);
    }
    else {
        for (size_t i = 0; i < p->size; i++) {
            printf("(");
            PolyPrint(&(p->arr[i]).p);
            printf(",%d)", (p->arr[i]).exp);
            if (i != p->size - 1) {
                printf("+");
            }
        }
    }
}

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

 static bool CorrectAfterPlus(const char **line, int *parenthesis_counter);

/**
 * Funkcja sprawdza, czy linia zawierająca wielomian, nie zaczynająca się
 * znakiem '(' opisuje poprawny wielomian będący współczynnikiem.
 * Jest wywoływana w CorrectPoly jeśli pierwszy znak linii
 * nie jest znakiem '('.
 */
static bool CorrectPolyFromCoeff(const char **line, bool first_function_call) {
    if (IsDigitOrMinus(**line) && first_function_call) {
        char *end;
        errno = 0;
        poly_coeff_t coeff = strtol(*line, &end, DECIMAL_BASE);
        if (*end == 0 || *end == ENDL) {
            if (coeff == LONG_MIN || coeff == LONG_MAX) {
                if (errno == ERANGE) return false;
            }
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

/**
 * Funkcja sprawdza poprawność ciągu znaków zawierającego wykładnik jednomianu.
 * Po wykładniku musi wystąpić znak ')',
 * Wczytany wykładnik musi mieścić się w przedziale <0, INT_MAX>.
 */
static bool CorrectExp(const char **line, char **end) {
    long exp;
    exp = strtol(*line, end, DECIMAL_BASE);
    if (**end != CLOSE_PARENTHESIS || exp > INT_MAX || exp < 0) {
        return false;
    }
    else return true;
}

/**
 * Funkcja sprawdza poprawność ciągu znaków zawierającego współczynnik jednomianu.
 * Po współczynniku musi wystąpić znak ',',
 * Wczytany współczynnik musi mieścić się w przedziale <LONG_MIN, LONG_MAX>.
 */
static bool CorrectCoeff(const char *line, char **end) {
    poly_coeff_t coeff;
    errno = 0;
    coeff = strtol(line, end, DECIMAL_BASE);
    if (**end != COMMA) {
        return false;
    }
    else if ((coeff == LONG_MAX || coeff == LONG_MIN) && errno == ERANGE) {
        return false;
    }
    else {
        return true;
    }
}

/**
 * Funkcja sprawdza poprawność linii zawierającej wielomian po wystąpieniu znaku ','.
 * Po znaku ',' musi wystąpić cyfra, która jest pierwsza cyfrą
 * wykładnika jednomianu. Funkcja rozważa pojawienie się znaku '+',
 * znaków końca lini, oraz znaków innych niż ',' i powyższe.
 * Jeśli na taki napotka zwraca false.
 */
static bool CorrectAfterComma(const char **line, char **end, int *parenthesis_counter) {
    while (**line == COMMA) {
        *line += 1;
        if (!IsDigit(**line)) {
            return false;
        }
        else if (!CorrectExp(line, end)) {
            return false;
        }
        else {
            *parenthesis_counter -= 1;
            *line = *end;
            *line += 1;
            if (**line == 0 || **line == ENDL) {
                if (*parenthesis_counter == 0) return true;
                else return false;
            }
            else if (**line == PLUS) {
                return CorrectAfterPlus(line, parenthesis_counter);
            }
            else {
                if (**line != COMMA) {
                    return false;
                }
            }
        }
    }
    return false;
}

/**
 * Funkcja sprawdza, czy linia opisuje poprawny wielomian.
 * Zlicza nawiasy otwierające, które napotka podczas parsowania linii.
 * Odejmuje od nich nawiasy zamykające.
 */
static bool CorrectPoly(const char *line, int *parenthesis_counter, bool first_function_call) {
    char *end;
    // Sprawdzam, czy wielomian będzie współczynnikiem.
    if (*line != OPEN_PARENTHESIS) {
        return CorrectPolyFromCoeff(&line, first_function_call);
    }
    else {
        // Pomijam nawiasy.
        while (*line == OPEN_PARENTHESIS) {
            *parenthesis_counter += 1;
            line++;
        }
        if (!IsDigitOrMinus(*line)) {
            return false;
        }
        else if (!CorrectCoeff(line, &end)) {
            return false;
        }
        else { // Teraz *end to znak ','.
            line = end;
            line++;
            if (!IsDigit(*line)) { // Pierwsza cyfra wykładnika.
                return false;
            }

            if (!CorrectExp(&line, &end)) {
                return false;
            }
            else {
                *parenthesis_counter -= 1;
                line = end;
                line++; // Pierwszy znak po zamykajacym nawiasie.
                if (*line == 0 || *line == ENDL) {
                    if (*parenthesis_counter == 0) return true;
                    else return false;
                }
                else if (*line == COMMA) {
                    if (CorrectAfterComma(&line, &end, parenthesis_counter)) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
                else if (*line == PLUS) {
                    return CorrectAfterPlus(&line, parenthesis_counter);
                }
                else {
                    return false;
                }
            }
        }
    }
}

/**
 * Funkcja zwraca true, jeśli wielomian po wystąpieniu znaku '+' jest poprawny.
 */
static bool CorrectAfterPlus(const char **line, int *parenthesis_counter) {
    *line += 1;
    bool correct = CorrectPoly(*line, parenthesis_counter, false);
    return correct;
}

Mono MonoFromString(const char **line) {
    Mono result;
    result.p = PolyFromString(line);
    *line += 1; // Pomijam znak ','.

    char *end;
    result.exp = strtol(*line, &end, DECIMAL_BASE);
    *line = end;
    *line += 1; // Pomijam znak ')'.
    return result;
}

Poly PolyFromString(const char **line) {
    // Jeżeli pierwszy znak jest cyfrą lub znakiem '-' , czytam współczynnik
    // i tworzę z niego wielomian.
    if (IsDigitOrMinus(**line)) {
        char *end;
        poly_coeff_t coeff = strtol(*line, &end, DECIMAL_BASE);
        *line = end;
        return C(coeff);
    }
    else{
        size_t arr_size = INITIAL_SIZE;
        size_t monos_counter = 0;
        Mono *arr = malloc(arr_size * sizeof(Mono));
        if (arr == NULL) exit(1);

        while (**line != 0 && **line != ENDL && **line != COMMA) {
            // Pomijam znak '('.
            *line += 1;
            if (monos_counter + 1 == arr_size) {
                arr_size *= 2;
                arr = realloc(arr, arr_size * sizeof(Mono));
                if (arr == NULL) exit(1);
            }
            arr[monos_counter] = MonoFromString(line);
            monos_counter++;
            if (**line == PLUS) *line += 1;
        }
        Poly result = PolyAddMonos(monos_counter, arr);
        free(arr);
        return result;
    }
}

/**
 * Funkcja sprawdza, czy linia jest pusta.
 */
static bool IsEmpty(const char *line) {
    if (line[0] == 0 || line[0] == ENDL) return true;
    else return false;
}

/**
 * Funkcja sprawdza, czy znak jest literą.
 */
static bool IsLetter(char c) {
    if ((c >= SMALL_A && c <= SMALL_Z) || (c >= CAPITAL_A && c <= CAPITAL_Z))
        return true;
    else
        return false;
}

/**
 * Funkcja przetwarza pojedynczą linię z wejścia. Pomija linie puste i komentarze.
 * Komunikaty o błędach wypisuje na standardowe wyjście diagnostyczne.
 */
static void ProcessLine(const char *line, Stack **Polynomials, int line_number, size_t line_size) {
    if (IsEmpty(line)) {
        return;
    }
    char first_char = line[0];
    if (first_char == COMMENT) {
        return;
    }
    // Sprawdzamy poprawność wielomianu, jeżeli jest poprawny to wkładamy go na stos.
    else if (first_char == OPEN_PARENTHESIS || IsDigitOrMinus(first_char)) {
        int parenthesis_counter = 0;
        if (CorrectPoly(line, &parenthesis_counter, true)) {
            Poly p = PolyFromString(&line);
            Push(Polynomials, p);
        }
        else {
            fprintf(stderr, "ERROR %d WRONG POLY\n", line_number);
        }
    }
    else {
        if (!isspace(first_char)) {
            // Polecenie musi zaczynać się literą, więc jeśli linia nie zaczyna
            // się literą, wypisujemy komunikat o błędzie.
            if (!IsLetter(first_char)) {
                fprintf(stderr, "ERROR %d WRONG POLY\n", line_number);
            }
            else {
            // Pobieramy pierwsze słowo i próbujemy wykonać instrukcję,
            // którą opisuje.
                char *first_word;
                sscanf(line, "%ms", &first_word);
                ExecuteInstruction(Polynomials, first_word, line, line_number, line_size);
                free(first_word);
            }
        }
        // Polecenie musi zaczynać się literą, więc jeśli linia zaczyna się
        // białym znakiem to wypisujemy komunikat o błędzie.
        else {
            fprintf(stderr, "ERROR %d WRONG POLY\n", line_number);
        }
    }
}

/**
 * Funkcja rozpoznaje, jaki błąd powinna wypisać na standardowe
 * wyjście diagnostyczne. Sprawdza czy linia zawiera polecenie, a po nim
 * biały znak, wtedy, w zależności od polecenia wypisuje konkretny komunikat
 * o błędzie.
 */
static void PrintConcreteErrors(size_t length, char *current_line, int line_number) {
    switch(current_line[0]) {
        case AT_FIRST_CHAR:
            if (length < AT_DIGIT_IDX) {
                fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            }
            else {
                if (current_line[IDX_1] == AT_SECOND_CHAR &&
                    isspace(current_line[AT_DIGIT_IDX - 1])) {
                    fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
                }
                else {
                    fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
                }
            }
            break;
        case DEG_BY_FIRST_CHAR:
            if (length < DEG_BY_DIGIT_IDX) {
                fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            }
            else {
                if (current_line[IDX_1] == DEG_BY_SECOND_CHAR &&
                    current_line[IDX_2] == DEG_BY_THIRD_CHAR &&
                    current_line[IDX_3] == DEG_BY_FOURTH_CHAR &&
                    current_line[IDX_4] == DEG_BY_FIFTH_CHAR &&
                    current_line[IDX_5] == DEG_BY_SIXTH_CHAR &&
                    isspace(current_line[DEG_BY_DIGIT_IDX - 1])) {
                    fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
                }
                else {
                    fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
                }
            }
            break;
        case COMPOSE_FIRST_CHAR:
            if (length < COMPOSE_DIGIT_IDX ) {
                fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            }
            else {
                if (current_line[IDX_1] == COMPOSE_SECOND_CHAR &&
                    current_line[IDX_2] == COMPOSE_THIRD_CHAR &&
                    current_line[IDX_3] == COMPOSE_FOURTH_CHAR &&
                    current_line[IDX_4] == COMPOSE_FIFTH_CHAR &&
                    current_line[IDX_5] == COMPOSE_SIXTH_CHAR &&
                    current_line[IDX_6] == COMPOSE_SEVENTH_CHAR &&
                    isspace(current_line[COMPOSE_DIGIT_IDX - 1])) {
                    fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
                }
                else {
                    fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
                }
            }
            break;
        default:
            fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            break;
    }
}

/**
 * Funkcja tworzy stos, po czym po kolei pobiera linie z wejścia,
 * na każdej z nich wykonuje ProcessLine. Po przetworzeniu linii zwalnia
 * pozostałą pamięć.
 */
int main(void) {
    char *current_line = NULL;
    int i = 0;
    size_t size;
    size_t line_size = getline(&current_line, &size, stdin);
    if (current_line == NULL) exit(1);
    Stack *Polynomials;
    Init(&Polynomials);
    while ((int)line_size != -1) {
        if (strlen(current_line) == line_size) {
            ProcessLine(current_line, &Polynomials, i + 1, line_size);
        }
        else {
            if (IsLetter(current_line[0])) {
                size_t length = strlen(current_line);
                PrintConcreteErrors(length, current_line, i + 1);
            }
            else {
                fprintf(stderr, "ERROR %d WRONG POLY\n", i + 1);
            }
        }
        i++;
        line_size = getline(&current_line, &size, stdin);
        if (current_line == NULL) exit(1);
    }
    free(current_line);
    Poly p;
    while (!Empty(Polynomials)) {
        p = Pop(&Polynomials);
        PolyDestroy(&p);
    }
    free(Polynomials);
    return 0;
}
