/** @file
  Implementacja listowa stosu.

  @author Mikołaj Szkaradek
  @date 2021
*/

#include "poly.h"
#include "stack.h"
#include <stdlib.h>

void Init(Stack **s) {
    *s = NULL;
}

bool Empty(Stack *s) {
    return (s == NULL);
}

void Push(Stack **s, Poly p) {
    Stack *temp;
    temp = malloc(sizeof(Stack));
    if (temp == NULL) exit(1);
    temp->next = *s;
    temp->v = p;
    *s = temp;
}

Poly Pop(Stack **s) {
    Stack *temp = *s;
    Poly p;
    p = (*s)->v;
    *s = (*s)->next;
    free(temp);
    return(p);
}

Poly Top(Stack *s) {
    return (s->v);
}
