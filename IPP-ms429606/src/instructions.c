/** @file
  Implementacja instrukcji kalkulatora
  wielomianów rzadkich wielu zmiennych.

  @author Mikołaj Szkaradek
  @date 2021
*/

#include "poly.h"
#include "stack.h"
#include "instructions.h"
#include <stdlib.h>
#include <stdio.h>

// Identyfikatory instrukcji.
#define ADD_ID 'A'              ///< Stała na identyfikator instrukcji ADD.
#define SUB_ID 'S'              ///< Stała na identyfikator instrukcji SUB.
#define MUL_ID 'M'              ///< Stała na identyfikator instrukcji MUL.

#define INITIAL_SIZE 4          ///< Stała na początkowy rozmiar tablicy.

void Zero(Stack **Polynomials) {
    Push(Polynomials, PolyZero());
}

void IsCoeff(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Top(*Polynomials);
        if (PolyIsCoeff(&p)) {
            printf("1\n");
        }
        else {
            printf("0\n");
        }
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void IsZero(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Top(*Polynomials);
        if (PolyIsZero(&p)) printf("1\n");
        else printf("0\n");
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void Clone(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly top = Top(*Polynomials);
        Poly p = PolyClone(&top);
        Push(Polynomials, p);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void AddSubOrMul(Stack **Polynomials, int line_number, char ID) {
    if (!Empty(*Polynomials)) {
        Poly p = Pop(Polynomials);
        if (!Empty(*Polynomials)) {
            Poly q = Pop(Polynomials);
            Poly result;
            if (ID == ADD_ID) result = PolyAdd(&p, &q);
            else if (ID == SUB_ID) result = PolySub(&p,&q);
            else result = PolyMul(&p, &q);
            Push(Polynomials, result);
            PolyDestroy(&p);
            PolyDestroy(&q);
        }
        else {
            Push(Polynomials, p);
            fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
        }
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void Neg(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Pop(Polynomials);
        Poly p_neg = PolyNeg(&p);
        Push(Polynomials, p_neg);
        PolyDestroy(&p);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void IsEq(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Pop(Polynomials);
        if (!Empty(*Polynomials)) {
            Poly q = Top(*Polynomials);
            if (PolyIsEq(&p, &q)) printf("1\n");
            else printf("0\n");
        }
        else {
            fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
        }
        Push(Polynomials, p);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void Deg(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Top(*Polynomials);
        poly_exp_t deg = PolyDeg(&p);
        printf("%d\n", deg);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void PopPoly(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Pop(Polynomials);
        PolyDestroy(&p);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void Print(Stack **Polynomials, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Top(*Polynomials);
        PolyPrint(&p);
        printf("\n");
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void DegBy(Stack **Polynomials, unsigned long idx, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Top(*Polynomials);
        poly_exp_t deg = PolyDegBy(&p, idx);
        printf("%d\n", deg);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void At(Stack **Polynomials, long x, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly p = Pop(Polynomials);
        Poly at = PolyAt(&p, x);
        PolyDestroy(&p);
        Push(Polynomials, at);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}

void Compose(Stack **Polynomials, size_t count, int line_number) {
    if (!Empty(*Polynomials)) {
        Poly main_poly = Pop(Polynomials);

        size_t size = INITIAL_SIZE;
        Poly *compose_elems_temp = malloc(size * sizeof(Poly));
        if (compose_elems_temp == NULL) exit(1);

        size_t popped_polys = 0;
        size_t resizer = 0;
        for (size_t i = 0; i < count; i++) {
            if (!Empty(*Polynomials)) {
                Poly p = Pop(Polynomials);
                compose_elems_temp[i] = p;
                popped_polys++;
                resizer++;
                if (resizer == size) {
                    size *= 2;
                    resizer = 0;
                    compose_elems_temp = realloc(compose_elems_temp, size * sizeof(Poly));
                    if (compose_elems_temp == NULL) exit(1);
                }
            }
            else {
                for (size_t i = popped_polys; i > 0; i--) {
                    Push(Polynomials, compose_elems_temp[i - 1]);
                }
                Push(Polynomials, main_poly);
                free(compose_elems_temp);
                fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
                return;
            }
        }
        Poly *compose_elems = malloc(count *sizeof(Poly));
        if (compose_elems == NULL) exit(1);
        for (size_t i = 0; i < count; i++) {
            compose_elems[i] = compose_elems_temp[count - i - 1];
        }
        free(compose_elems_temp);
        Poly composed_poly = PolyCompose(&main_poly, count, compose_elems);
        PolyDestroy(&main_poly);
        for (size_t i = 0; i < count; i++) {
            PolyDestroy(&compose_elems[i]);
        }
        free(compose_elems);
        Push(Polynomials, composed_poly);
    }
    else {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
}
