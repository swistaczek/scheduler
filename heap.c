#include "heap.h"
// obsluga stosu

// initializacja stosu
// ustaw rozmiar stosu na zero i NULL na wszystkie wartosci
void heap_init(heap *h)
{
    int j;
    h->size = 0;
    for(j = 0; j < MAX_HEAP_SIZE; j++)
        h->a[j] = NULL;
    return;
}

// najwiekszy element ze stosu wg. przeslanej funkcji porownania
// Zapozyczone z ksiazki Introduction to Algorithms, 1990, Cormen, Leiserson, Rivest, and Stein,
// Rozdzial 7, Sekcja 7.2
void heapify(heap *h, int (*comp_func)(void*, void*), int i)
{
    void *temp = NULL;
    int largest,
        l = LEFT(i),
        r = RIGHT(i);
    if(l <= h->size && comp_func(h->a[l], h->a[i]) > 0)
        largest = l;
    else
        largest = i;
    if(r <= h->size && comp_func(h->a[r], h->a[largest]) > 0)
        largest = r;
    if(largest != i) {
        SWAP(h->a[i], h->a[largest], temp);
        heapify(h, comp_func, largest);
    }
    return;
}

// zbuduj stos
// Zapozyczone z ksiazki Introduction to Algorithms, 1990, Cormen, Leiserson, Rivest, and Stein,
// Rozdzial 7, Sekcja 7.3
void build_heap(heap *h, int (*comp_func)(void*, void*))
{
    int i;
    for(i = h->size/2; i > 0; i--)
        heapify(h, comp_func, i);
    return;
}

// maxymalna wartosc ze stosu
// Zapozyczone z ksiazki Introduction to Algorithms, 1990, Cormen, Leiserson, Rivest, and Stein,
// Rozdzial 7, Sekcja 7.5
void* heap_extract_max(heap *h, int (*comp_func)(void*, void*))
{
    if(h->size < 1)
        return NULL;
    void *max = h->a[1],
         *temp = NULL;
    SWAP(h->a[1], h->a[h->size], temp);
    h->size--;
    heapify(h, comp_func, 1);
    return max;
}

// wsadzanie na stos
// Zapozyczone z ksiazki Introduction to Algorithms, 1990, Cormen, Leiserson, Rivest, and Stein,
// Rozdzial 7, Sekcja 7.5
void heap_insert(heap *h, int (*comp_func)(void*, void*), void *key)
{
    int i = ++h->size;
    while(i > 1 && comp_func(h->a[PARENT(i)], key) < 0) {
        h->a[i] = h->a[PARENT(i)];
        i = PARENT(i);
    }
    h->a[i] = key;
    return;
}
