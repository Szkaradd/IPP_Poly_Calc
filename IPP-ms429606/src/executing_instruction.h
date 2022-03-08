/** @file
  Interfejs funkcji wykorzystywanych do wykonania
  instrukcji.

  @author Mikołaj Szkaradek
  @date 2021
*/

#ifndef __EXECUTING_INSTRUCTION_H__
#define __EXECUTING_INSTRUCTION_H__

#include "poly.h"
#include "stack.h"

/**
 * Funkcja sprawdza, czy pierwsze słowo jest którąś z instrukcji, jeśli tak to
 * ją wykonuje lub w przypadku instrukcji dwuargumentowych próbuje wykonać.
 * Jeżeli napotka problem wypisuje na standardowe wyjście diagonostyczne komunikat
 * z błędem.
 */
void ExecuteInstruction(Stack **Polynomials, char *instruction,
                        const char *const_line, int line_number, size_t line_size);

#endif /* __EXECUTING_INSTRUCTION__ */
