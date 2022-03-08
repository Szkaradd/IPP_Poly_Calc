/** @file
  Interfejs instrukcji kalkulatora
  wielomianów rzadkich wielu zmiennych.

  @author Mikołaj Szkaradek
  @date 2021
*/

#ifndef __INSTRUCTIONS_H__
#define __INSTRUCTIONS_H__

/**
 * Funkcja wstawia na wierzchołek stosu wielomian tożsamościowo równy 0.
 */
void Zero(Stack **Polynomials);

/**
 * Funkcja sprawdza, czy wielomian na wierzchołku stosu jest współczynnikiem.
 * Wypisuje na standardowe wyjście 0 lub 1. Jeżeli stos jest pusty to wypisuje
 * na standardowe wyjście diagnostyczne: ERROR w STACK UNDERFLOW\n.
 */
void IsCoeff(Stack **Polynomials, int line_number);

/**
 * Funkcja sprawdza, czy wielomian na wierzchołku stosu jest tożsamościowo równy 0.
 * Wypisuje na standardowe wyjście 0 lub 1. Jeżeli stos jest pusty to wypisuje
 * na standardowe wyjście diagnostyczne: ERROR w STACK UNDERFLOW\n.
 */
void IsZero(Stack **Polynomials, int line_number);

/**
 * Funkcja wstawia na stos kopię wielomianu z wierzchołka.
 * Jeżeli stos jest pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void Clone(Stack **Polynomials, int line_number);

/**
 * Funkcja dodaje/mnoży/odejmuje dwa wielomiany z wierzchu stosu,
 * usuwa je i wstawia na wierzchołek stosu ich sumę/iloczyn/różnicę,
 * w zależnośći od identyfikatora instrukcji (ID). Jeżeli stos jest
 * pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void AddSubOrMul(Stack **Polynomials, int line_number, char ID);

/**
 * Funkcja neguje wielomian na wierzchołku stosu. Jeżeli stos jest
 * pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void Neg(Stack **Polynomials, int line_number);

/**
 * Funkcja sprawdza, czy dwa wielomiany na wierzchu stosu są równe.
 * Wypisuje na standardowe wyjście 0 lub 1. Jeżeli stos jest
 * pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void IsEq(Stack **Polynomials, int line_number);

/**
 * Funkcja wypisuje na standardowe wyjście stopień wielomianu
 * (−1 dla wielomianu tożsamościowo równego zeru). Jeżeli stos jest
 * pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void Deg(Stack **Polynomials, int line_number);

/**
 * Funkcja usuwa wielomian z wierzchołka stosu. Jeżeli stos jest
 * pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void PopPoly(Stack **Polynomials, int line_number);

/**
 * Funkcja wypisuje na standardowe wyjście wielomian z wierzchołka stosu.
 * Jeżeli stos jest pusty to wypisuje na standardowe wyjście diagnostyczne:
 * ERROR w STACK UNDERFLOW\n.
 */
void Print(Stack **Polynomials, int line_number);

/**
 * Funkcja wypisuje na standardowe wyjście stopień wielomianu ze względu
 * na zmienną o numerze idx. Jeżeli stos jest pusty to wypisuje na
 * standardowe wyjście diagnostyczne: ERROR w STACK UNDERFLOW\n.
 */
void DegBy(Stack **Polynomials, unsigned long idx, int line_number);

/**
 * Funkcja wylicza wartość wielomianu w punkcie x, usuwa wielomian z wierzchołka
 * i wstawia na stos wynik operacji. Jeżeli stos jest pusty to wypisuje na
 * standardowe wyjście diagnostyczne: ERROR w STACK UNDERFLOW\n.
 */
void At(Stack **Polynomials, long x, int line_number);

/**
 * Funkcja wykonuje operacje składania wielomianu. Wstawia na stos wynik operacji.
 * Pobiera ze stosu po kolei wielomiany, które podstawimy za zmienne w wielomianie
 * głównym (pierwszym zdjętym ze stosu). Jeżeli w którymkolwiek momencie ściągania
 * wielomianów ze stosu, stos jest pusty, to odkładamy wszystkie z powrotem i
 * wypisujemy na standardowe wyjście diagnostyczne: ERROR w STACK UNDERFLOW\n.
 */
void Compose(Stack **Polynomials, size_t count, int line_number);

#endif /* __INSTRUCTIONS_H__ */

