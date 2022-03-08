/** @file
  Interfejs listowego stosu.

  @author Mikołaj Szkaradek
  @date 2021
*/

#ifndef __STACK_H__
#define __STACK_H__

/**
 * To jest struktura przechowująca stos.
 */
typedef struct Stack{
    /** Wielomian. */
    Poly v;
    /** Wskaźnik na kolejny element. */
    struct Stack *next;
}Stack;

/**
 * Funkcja inicjująca stos.
 */
void Init(Stack **s);

/**
 * Funkcja sprawdza, czy stos jest pusty.
 */
bool Empty(Stack *s);

/**
 * Funkcja wstawia wielomian na wierzchołek stosu.
 */
void Push(Stack **s, Poly p);

/**
 * Funkcja zdejmuje wielomian z wierzchołka stosu.
 */
Poly Pop(Stack **s);

/**
 * Funkcja podgląda wielomian na wierzchołku stosu.
 */
Poly Top(Stack *s);

#endif /* __STACK_H__ */
