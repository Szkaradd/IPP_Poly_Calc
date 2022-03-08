/** @file
  Implementacja Funkcji obsługujących działania na wielomianach wielu zmiennych.

  @author Mikołaj Szkaradek
  @date 2021
*/

#include "poly.h"
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_ARR_SIZE 4 ///<Stała na początkowy rozmiar tablicy.

void PolyDestroy(Poly *p) {
    assert(p != NULL);
    if (p->arr != NULL) {
        for (size_t i = 0; i < p->size; i++) {
            MonoDestroy(&(p->arr[i]));
        }
        free(p->arr);
    }
}

Poly PolyClone(const Poly *p) {
    assert(p != NULL);
    Poly clone;
    if (p->arr == NULL) {
        clone.arr = NULL;
        clone.coeff = p->coeff;
    }
    else {
        clone.size = p->size;
        clone.arr = malloc(clone.size * sizeof(Mono));
        if (clone.arr == NULL) exit(1);
        for (size_t i = 0; i < clone.size; i++) {
            clone.arr[i] = MonoClone(&(p->arr[i]));
        }
    }
    return clone;
}

/**
 * Jeżeli zajdzie taka potrzeba, zwiększa rozmiar tablicy jednomianów dwukrotnie.
 * Aktualizuje także obecny rozmiar tablicy.
 */
static void AddSizeIfNeeded(Mono **m, size_t res_size, size_t *current_size) {
    if (res_size == *current_size) {
        *current_size *= 2;
        *m = realloc(*m, *current_size * sizeof(Mono));
        if (*m == NULL) exit(1);
    }
}

/**
 * Funkcja pomocnicza do AddArraysOfMonos, kiedy przynajmniej jeden
 * indeks już osiągnął koniec tablicy, to uzupełnia tablicę *res
 * do końca, elementami tablicy, której indeks jeszcze nie osiągnał końca.
 */
static void FillRestOfArrays(Mono **res, Mono *p_arr, Mono *q_arr,
                             size_t p_arr_index, size_t q_arr_index, size_t res_arr_index,
                             size_t p_size, size_t q_size, size_t *res_size, size_t *current_size) {
    while (p_arr_index < p_size) {
        AddSizeIfNeeded(res, *res_size, current_size);

        (*res)[res_arr_index] = MonoClone(&p_arr[p_arr_index]);
        p_arr_index++;
        res_arr_index++;
        (*res_size)++;
    }
    while (q_arr_index < q_size) {
        AddSizeIfNeeded(res, *res_size, current_size);

        (*res)[res_arr_index] = MonoClone(&q_arr[q_arr_index]);
        q_arr_index++;
        res_arr_index++;
        (*res_size)++;
    }
}

/**
 * Funkcja tworzy tablicę jednomianów sumując jednomiany z tablic w, p i q.
 * Jest wywoływana kiedy p->arr i q->arr są != NULL.
 * Ustawia także rozmiar tablicy wynikowej.
 * Jeśli miałby być równy 0, zwraca Wielomian zerowy.
 * W przeciwnym wypadku zwraca wielomian, z ustawioną tablicą jednomianów.
 */
static Poly AddArraysOfMonos(Mono *p_arr, Mono *q_arr, size_t p_size,
                             size_t q_size, size_t *res_size) {
    Poly result;
    size_t current_size = INITIAL_ARR_SIZE;
    Mono *res = malloc(current_size * sizeof(Mono));
    if (res == NULL) exit(1);
    // Tworzymy 3 indexy jeden będzie poruszać się po tablicy wynikowej,
    // pozostałe po p_arr i q_arr.
    size_t res_arr_index = 0;
    size_t p_arr_index = 0;
    size_t q_arr_index = 0;
    while (p_arr_index < p_size && q_arr_index < q_size) {
        AddSizeIfNeeded(&res, *res_size, &current_size);
        // Teraz bedziemy sprawdzac relacje między wykładnikami jednomianów w p_arr i q_arr.
        if (p_arr[p_arr_index].exp == q_arr[q_arr_index].exp) {
            Poly q_neg = PolyNeg(&(q_arr[q_arr_index].p));
            // Tworzymy wielomian przeciwny do wielomianu q_arr[q_arr_index].p.
            // Jeśli jest równy wielomianowi p_arr[p_arr_index].p to znaczy,
            // że dają 0 po dodaniu, więc w takim wypadku nie chcemy nic dodawać do
            // tablicy res. i nie aktualizujemy indeksu res.
            if (PolyIsEq(&q_neg, &(p_arr[p_arr_index].p))) {
                p_arr_index++;
                q_arr_index++;
            }
            else {
            // Sumę wielomianów wstawiamy do res, zwiekszamy res_size i wszystkie indeksy.
                res[res_arr_index].exp = p_arr[p_arr_index].exp;
                res[res_arr_index].p = PolyAdd(&(p_arr[p_arr_index].p), &(q_arr[q_arr_index].p));
                p_arr_index++;
                q_arr_index++;
                res_arr_index++;
                (*res_size)++;
            }
            PolyDestroy(&q_neg);
        }
        // W przypadkach poniżej do res klonujemy ten jednomian, który ma mniejszy wykładnik.
        else if (p_arr[p_arr_index].exp < q_arr[q_arr_index].exp) {
            res[res_arr_index] = MonoClone(&p_arr[p_arr_index]);
            p_arr_index++;
            res_arr_index++;
            (*res_size)++;
        }
        else {
            res[res_arr_index] = MonoClone(&q_arr[q_arr_index]);
            q_arr_index++;
            res_arr_index++;
            (*res_size)++;
        }
    }

    FillRestOfArrays(&res, p_arr, q_arr, p_arr_index, q_arr_index,
                     res_arr_index, p_size, q_size, res_size, &current_size);
    if (*res_size == 0) {
        result = PolyZero();
        free(res);
        return result;
    }
    else {
        result.arr = res;
        result.size = *res_size;
        return result;
    }
}

/**
 * Funkcja odpowiada za przypadek dodania wielomianu do wspołczynnika.
 * Jest wywoływana kiedy dokładnie 1 z p->arr i q->arr jest != NULL.
 * Zwraca wielomian z ustawiona tablica jednomianów i rozmiarem.
 */
static Poly AddArrayToCoeff(const Mono *p_arr, size_t p_size, const Poly *q, size_t *res_arr_size) {
    Poly result;
    poly_coeff_t q_coeff = q->coeff;
    Mono *res;
    if (q_coeff == 0) {
    // Res staje sie poprostu kopią p_arr.
        res = malloc(p_size * sizeof(Mono));
        if (res == NULL) exit(1);
        *res_arr_size = p_size;
        for (size_t i = 0; i < p_size; i++) {
            res[i] = MonoClone(&p_arr[i]);
        }
    }
    else {
        if (p_arr[0].exp == 0) {
            res = malloc(p_size * sizeof(Mono));
            if (res == NULL) exit(1);
            if (PolyIsCoeff(&p_arr[0].p)) {
                if (p_arr[0].p.coeff == -q_coeff) {
                // Zwrócimy wielomian, składający sie z pozostałych jednomianów z p_arr.
                    *res_arr_size = p_size - 1;
                    for (size_t i = 1; i < p_size; i++) {
                        res[i - 1] = MonoClone(&p_arr[i]);
                    }
                }
                else {
                // Res stanie się kopią p_arr z ta różnicą, że na indeksie 0 dodajemy q_coeff.
                    *res_arr_size = p_size;
                    res[0].exp = 0;
                    res[0].p = PolyFromCoeff(q_coeff + p_arr[0].p.coeff);
                    for (size_t i = 1; i < p_size; i++) {
                        res[i] = MonoClone(&p_arr[i]);
                    }
                }
            }
            // Do zerowego elementu p_arr dodajemy PolyAdd wielomian q.
            // Res stanie się kopią p_arr za wyjątkiem zerowego indeksu.
            else {
                *res_arr_size = p_size;
                res[0].exp = p_arr[0].exp;
                res[0].p = PolyAdd(q, &p_arr[0].p);
                for (size_t i = 1; i < p_size; i++) {
                    res[i] = MonoClone(&p_arr[i]);
                }
            }
        }
        else {
            res = malloc((p_size + 1) * sizeof(Mono));
            if (res == NULL) exit(1);
            res[0].exp = 0;
            res[0].p.arr = NULL;
            res[0].p.coeff = q_coeff;
            for (size_t i = 0; i < p_size; i++) {
                res[i + 1] = MonoClone(&p_arr[i]);
            }
            *res_arr_size = p_size + 1;
        }
    }
    result.arr = res;
    result.size = *res_arr_size;
    return result;
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    Poly res;
    if (p->arr == NULL && q->arr == NULL) {
        poly_coeff_t new_coeff = p->coeff + q->coeff;
        return PolyFromCoeff(new_coeff);
    }
    else if (p->arr == NULL && q->arr != NULL) {
        size_t res_size = 0;
        res = AddArrayToCoeff(q->arr, q->size, p, &res_size);
    }
    else if (p->arr != NULL && q->arr == NULL) {
        size_t res_size = 0;
        res = AddArrayToCoeff(p->arr, p->size, q, &res_size);
    }
    else {
        size_t res_size = 0;
        res = AddArraysOfMonos(p->arr, q->arr, p->size, q->size, &res_size);
    }
    // Jeżeli res.size = 1 oraz jedyny jednomian jest poprostu współczynnikiem,
    // zwracamy wielomian będący tym współczynnikiem.
    if (res.arr != NULL && res.size == 1 && res.arr[0].exp == 0 && res.arr[0].p.arr == NULL) {
        poly_coeff_t ncoeff = res.arr[0].p.coeff;
        PolyDestroy(&res);
        return PolyFromCoeff(ncoeff);
    }
    else {
        return res;
    }
}

/**
 * Funkcja pomocnicza do qsorta, porównuje 2 wielomiany ze wzgledu na wykładnik.
 */
static int CompareMonosByExp(const void *a, const void *b) {
    Mono mono1 = *(Mono*)a;
    Mono mono2 = *(Mono*)b;

    if (mono1.exp < mono2.exp) return -1;
    else if (mono1.exp == mono2.exp) return 0;
    else return 1;
}

/**
 * Funkcja usuwa jednomiany zerowe (o wspolczynniku = 0) z tablicy wielomianu.
 */
static void PolyRemoveZeros(Poly *p) {
    /* Licznik znalezionych zer - każdy element w tablicy będziemy przesuwać
       o ilość zer go poprzedzających. */
    if (PolyIsCoeff(p)) return;
    size_t counter = 0;

    for (size_t i = 0; i < p->size; i++) {
        if (PolyIsZero(&p->arr[i].p)) {
            counter++;
        }
        else if (counter > 0) { // Przesuwamy element i-ty o ilość znalezionych zer.
            p->arr[i - counter] = p->arr[i];
        }
    }
    p->size -= counter;
    if (p->size == 0) { // Wszystkie elementy były zerami.
        free(p->arr);
        p->arr = NULL;
        p->coeff = 0;
    }
}


/**
 * Funkcja pomocnicza do PolyAddMonos, tworzy wielomian z jednomianu.
 * Wywoływana w PolyFromMonos gdy liczba jednomianów = 1.
 */
static Poly PolyFromMono(Mono m) {
    PolyRemoveZeros(&(m.p)); // Usuwamy wielomiany o zerowych współczynnikach.
    if (m.exp == 0 && PolyIsCoeff(&m.p)) {
        poly_coeff_t new_coeff = m.p.coeff;
        return PolyFromCoeff(new_coeff);
    }
    else if (PolyIsZero(&m.p)) {
        return PolyZero();
    }
    else {
        Poly result;
        result.arr = malloc(sizeof(Mono));
        if (result.arr == NULL) exit(1);
        result.size = 1;
        result.arr[0] = m;

        if (!PolyIsCoeff(&m.p) && m.p.size == 1) {
            if (m.p.arr[0].exp == 0 && PolyIsCoeff(&(m.p.arr[0].p))) {
                poly_coeff_t coeff = m.p.arr[0].p.coeff;
                MonoDestroy(&m);
                result.arr[0].p = PolyFromCoeff(coeff);
            }
        }

        return result;
    }
}

/**
 * Tworzy wielomian z posortowanej tablicy jednomianów. Przejmuje na własność
 * pamięć wskazywaną przez new_monos i jej zawartość. Może dowolnie modyfikować
 * zawartość tej pamięci.
 */
static Poly PolyFromMonos(size_t count, Mono *new_monos) {
    Poly res;
    if (count == 1) {
        res = PolyFromMono(new_monos[0]);
        free(new_monos);
        return res;
    }
    else {
        Mono *newer_monos = malloc(count * sizeof(Mono));
        if (newer_monos == NULL) exit(1);

        size_t j = 0;
        for (size_t i = 0; i < count - 1; i++) {
            if (new_monos[i].exp == new_monos[i + 1].exp) {
                Poly helper = PolyAdd(&new_monos[i].p, &new_monos[i + 1].p);
                PolyDestroy(&new_monos[i].p);
                PolyDestroy(&new_monos[i + 1].p);
                if (!PolyIsZero(&helper)) {
                    newer_monos[j].exp = new_monos[i].exp;
                    newer_monos[j].p = helper;
                    i += 2;
                    // Aby i-ty jednomian był pierwszym nie sprawdzonym wcześniej.
                    if (i <= count - 1) {
                        while (i < count && new_monos[i].exp == newer_monos[j].exp) {
                        // Dopóki wykładniki kolejnych jednomianów są takie same, dodaję
                        // do newer_monos[j].p każdy kolejny wielomian jednomianu.
                            Poly helper2 = newer_monos[j].p;
                            newer_monos[j].p = PolyAdd(&helper2, &new_monos[i].p);
                            PolyDestroy(&helper2);
                            PolyDestroy(&new_monos[i].p);
                            i++;
                        }
                        if (PolyIsZero(&newer_monos[j].p)) {
                        // Zmniejszam 'j' i niszczę wielomian tak aby dalej 'j' było takie samo.
                            PolyDestroy(&newer_monos[j].p);
                            j--;
                        }
                    }
                    i--;
                    j++;
                }
                // Zwiększam 'i' o 1, żeby w pętli for przejść o 2 do przodu, tak aby
                // nie sprawdzać tych dwóch jednomianów co wcześniej.
                else i++;
            }
            // Jeśli 2 kolejne jednomiany nie mają tego samego wykładnika to i-ty jednomian
            // z new_monos przepisujemy na j-te miejsce w newer_monos.
            else {
                newer_monos[j] = new_monos[i];
                j++;
            }
        }
        // Skrajne przypadki:
        // Możliwe jest, że nie przepisaliśmy ostatniego elementu tablicy jednomianów.
        // Dzieje się tak gdy jego wykładnik jest różny od przedostatniego, nie
        // wchodzimy wtedy w pętle for dla ostatniego jednomianu. W takim wypadku
        // wpisujemy go do newer_monos poniżej.
        if (new_monos[count - 1].exp != new_monos[count - 2].exp) {
            newer_monos[j] = new_monos[count - 1];
            j++;
        }
        // Jeżeli j jest równe zero, to wszystkie jednomiany się wyzerowały.
        if (j == 0) {
            free(new_monos);
            free(newer_monos);
            return PolyZero();
        }
        // Jeżeli j == 1 i jedyny jednomian w newer_monos ma wykładnik 0 oraz jego wielomian
        // jest wspołczynnikiem to chcemy zwrocić wielomian będący tym współczynnikiem.
        if (j == 1 && newer_monos[0].exp == 0 && PolyIsCoeff(&newer_monos[0].p)) {
            poly_coeff_t ncoeff = newer_monos[0].p.coeff;
            free(new_monos);
            free(newer_monos);
            return PolyFromCoeff(ncoeff);
        }
        free(new_monos);
        // Na wszelki wypadek usuwamy wszystkie wielomiany,
        // które mogłyby mieć zerowy wsþółczynnik.
        for (size_t i = 0; i < j; i++) {
            PolyRemoveZeros(&newer_monos[i].p);
        }
        res.size = j;
        res.arr = newer_monos;
        return res;
    }
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count == 0 || monos == NULL) {
        return PolyZero();
    }

    Mono *new_monos = malloc(count * sizeof(Mono));
    if (new_monos == NULL) exit(1);

    for (size_t i = 0; i < count; i++) {
        new_monos[i] = monos[i];
    }
    qsort(new_monos, count, sizeof(Mono), CompareMonosByExp);
    return PolyFromMonos(count, new_monos);
}

Poly PolyOwnMonos(size_t count, Mono *monos) {
    if (count == 0 || monos == NULL) {
        free(monos);
        return PolyZero();
    }
    qsort(monos, count, sizeof(Mono), CompareMonosByExp);
    return PolyFromMonos(count, monos);
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    if (count == 0 || monos == NULL) {
        return PolyZero();
    }
    Mono *new_monos = malloc(count * sizeof(Mono));
    if (new_monos == NULL) exit(1);

    for (size_t i = 0; i < count; i++) {
        new_monos[i] = MonoClone(&monos[i]);
    }
    qsort(new_monos, count, sizeof(Mono), CompareMonosByExp);
    return PolyFromMonos(count, new_monos);
}

/**
 * Funkcja pomocniczna do MulArrayAndCoeff
 * kiedy dwie liczby pomnożone przez siebie dają 0 to usuwa
 * jednomian z tego indeksu i przenosi reszte o index do tyłu
 */
static void ResizePolyAndRemoveMono(Poly *res, size_t index_to_remove) {
    MonoDestroy(&res->arr[index_to_remove]);
    for (size_t i = index_to_remove; i < res->size - 1; i++) {
        res->arr[i] = res->arr[i + 1];
    }
    res->size -= 1;
}

/**
 * Funkcja mnoży wielomian przez współczynnik
 */
static Poly PolyMulArrayAndCoeff(const Poly *p, const Poly *q) {
    poly_coeff_t q_coeff = q->coeff;
    Poly res = PolyClone(p);
    for (size_t i = 0; i < res.size; i++) {
        if (PolyIsCoeff(&res.arr[i].p)) {
            res.arr[i].p.coeff *= q_coeff;
            if (res.arr[i].p.coeff == 0) {
                ResizePolyAndRemoveMono(&res, i);
            }
        }
        else {
            Poly helper = res.arr[i].p;
            res.arr[i].p = PolyMul(&res.arr[i].p, q);
            PolyDestroy(&helper);
        }
    }
    if (res.size == 0) {
        PolyDestroy(&res);
        return PolyZero();
    }
    return res;
}

Poly PolyMul(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    if (p->arr == NULL && q->arr == NULL) {
        return PolyFromCoeff(p->coeff * q->coeff);
    }
    else if (p->arr != NULL && q->arr == NULL) {
        if (q->coeff == 0) return PolyFromCoeff(0);
        return PolyMulArrayAndCoeff(p, q);
    }
    else if (p->arr == NULL && q->arr != NULL) {
        if (p->coeff == 0) return PolyFromCoeff(0);
        return PolyMulArrayAndCoeff(q, p);
    }
    else {
    /*
    * Mnoży każdy wielomian jednomianu z każdym innym i tworzy z nich tablice
    * na tak stworzonej tablicy wykonuje funkcje PolyAddMonos.
    */
        size_t count = p->size * q->size;
        size_t monos_index = 0;
        Mono *monos = malloc(count * sizeof(Mono));
        if (monos == NULL) exit(1);
        for (size_t i = 0; i < p->size; i++) {
            for (size_t j = 0; j < q->size; j++) {
                monos[monos_index].p = PolyMul(&p->arr[i].p, &q->arr[j].p);
                monos[monos_index].exp = p->arr[i].exp + q->arr[j].exp;
                monos_index++;
            }
        }
        Poly res = PolyAddMonos(count, monos);
        free(monos);
        return res;
    }
}

Poly PolyNeg(const Poly *p) {
    assert(p != NULL);
    Poly neg = PolyFromCoeff(-1);
    Poly res = PolyMul(p, &neg);
    return res;
}

Poly PolySub(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    Poly q_neg = PolyNeg(q);
    Poly res = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    return res;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    // Korzystając z założenia, że tablica jednomianów jest posortowana po wykładnikach,
    // Dla var_idx = 0 stopniem wielomianu będzie wykładnik przy ostatnim elemencie tablicy
    // jednomianów. W pętli wywołujemy funkcje z var_idx - 1, kiedy osiągnie 0 zwraca stopień
    // według danej zmiennej.
    assert(p != NULL);
    if (p->arr == NULL) {
        if (p->coeff == 0) return -1;
        else return 0;
    }
    else {
        if (var_idx == 0) {
            return p->arr[p->size - 1].exp;
        }
        else {
            poly_exp_t max = 0;
            for (size_t i = 0; i < p->size; i++) {
                poly_exp_t current = PolyDegBy(&p->arr[i].p, var_idx - 1);
                if (current > max) {
                    max = current;
                }
            }
            return max;
        }
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    // Funkcja bada stopień każdego jednomianu rekurencyjnie.
    // Maksymalny znaleziony stopień to stopień wielomianu.
    assert(p != NULL);
    if (PolyIsZero(p)) return -1;
    else if (p->arr == NULL) return 0;
    else {
        poly_exp_t max_exp = 0;;
        poly_exp_t curr_exp;
        for (size_t i = 0; i < p->size; i++) {
            curr_exp = MonoGetExp(&(p->arr[i]));
            Poly temp = p->arr[i].p;

            if (temp.arr == NULL) {
                if (curr_exp > max_exp) {
                    max_exp = curr_exp;
                }
            }
            else {
                curr_exp += PolyDeg(&temp);
                if (curr_exp > max_exp) {
                    max_exp = curr_exp;
                }
            }
        }
        return max_exp;
    }
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    // Jeżeli oba wielomiany są współczynnikami to sprawdzam czy są one równe.
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        if (p->coeff == q->coeff) {
            return true;
        }
        else return false;
    }
    // Jeżeli oba wielomiany nie są współczynnikami:
    else if (p->arr != NULL && q->arr != NULL) {
        if (p->size != q->size) return false;
        else {
            // Sprawdzam czy wykładniki są takie same.
            for (size_t i = 0; i < p->size; i++) {
                if (p->arr[i].exp != q->arr[i].exp) {
                    return false;
                }
                else {
                    // Sprawdzam czy wielomiany są takie same.
                    if (!PolyIsEq(&p->arr[i].p, &q->arr[i].p)) {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    else return false;
}

/**
 * Podnosi x do potęgi exp.
 */
static poly_coeff_t Power(poly_coeff_t x, poly_exp_t exp) {
    poly_coeff_t result = 1;

    while (exp > 0) {
        if (exp % 2 == 1) result *= x;

        x *= x;
        exp /= 2;
    }
    return result;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    assert(p != NULL);
    if (x == 0 && p->arr != NULL) {
        if (p->arr[0].exp != 0) { // 0^0 = 1, wiec sprawdzamy ten warunek.
            return PolyZero();
        }
        else {
            return PolyClone(&p->arr[0].p);
        }
    }
    if (p->arr == NULL) {
        return PolyFromCoeff(p->coeff);
    }
    else {
        Poly res = PolyZero();
        for (size_t i = 0; i < p->size; i++) {
            poly_coeff_t x_power_exp = Power(x, p->arr[i].exp); // x^exp
            Poly x_power_exp_poly = PolyFromCoeff(x_power_exp); // C(x^exp)
            // Mnożymy x^exp przez wielomian, który był w
            // tablicy jednomianów na i-tym miejscu.
            Poly multiplier = PolyMul(&x_power_exp_poly, &p->arr[i].p);
            PolyDestroy(&x_power_exp_poly);
            Poly helper = res;
            res = PolyAdd(&helper, &multiplier);
            PolyDestroy(&helper);
            PolyDestroy(&multiplier);
        }
        return res;
    }
}

/**
 * Podnosi wielomian do potęgi power, korzystając z szybkiego potęgowania.
 */
static Poly PolyPower(const Poly *p, poly_exp_t power) {
    Poly result = PolyFromCoeff(1);
    Poly multiplier = PolyClone(p);
    Poly new_result;
    Poly new_multiplier;

    while (power > 0) {
        if (power % 2 == 1) {
            new_result = PolyMul(&result, &multiplier);
            PolyDestroy(&result);
            result = new_result;
        }

        new_multiplier = PolyMul(&multiplier, &multiplier);
        PolyDestroy(&multiplier);
        multiplier = new_multiplier;
        power /= 2;
    }
    PolyDestroy(&multiplier);
    return result;
}

/**
 * Funkcja zakłada, że wszystkie zmienne wielomianu są równe 0.
 * Sumuje te współczynniki, przy którychw ykładnik jest równy zero.
 * Zwraca wynik otrzymanej sumy.
 */
static poly_coeff_t PolyComposeIfKIsZero(const Poly *p) {
    if (PolyIsCoeff(p)) return p->coeff;
    else {
        poly_coeff_t res = 0;
        for (size_t i = 0; i < p->size; i++) {
            if (p->arr[i].exp == 0) {
                res += PolyComposeIfKIsZero(&(p->arr[i].p));
            }
        }
        return res;
    }
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff);
    }
    else {
        if (k > 0) {
            Poly new_result;
            Poly result = PolyZero();
            Poly composed_coeff;
            Poly composed_poly;
            for (size_t j = 0; j < p->size; j++) {
                // q^exp. Podnosimy nasz wielomian do wykładnika jednomianu.
                Poly power_poly = PolyPower(q, p->arr[j].exp);
                // Składamy współczynnik wielomianu (czyli wielomian jednomianu).
                composed_coeff = PolyCompose(&(p->arr[j].p), k - 1, q + 1);
                // Mnożymy otrzymany wielomian przez wielomian jednomianu.
                composed_poly = PolyMul(&power_poly, &composed_coeff);
                PolyDestroy(&composed_coeff);
                // Dodajemy do całego wyniku otrzymany wyżej wielomian.
                new_result = PolyAdd(&result, &composed_poly);
                PolyDestroy(&result);
                result = new_result;
                PolyDestroy(&power_poly);
                PolyDestroy(&composed_poly);
            }
            return result;
        }
        else {
            poly_coeff_t coeff = PolyComposeIfKIsZero(p);
            return PolyFromCoeff(coeff);
        }
    }
}

