#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "heap.h"

#define TRUE  1
#define FALSE 0

typedef struct {
    int id,         // id zadania 
        arrive,     // czas przybycia
        burst,      // pozostaly czas dzialania
        waiting,    // suma czasow czekania
        start,      // czas rozpcozecia
        end,        // czas zakonczenia
        priority,   // priorytet
        service;    // najaktualniejszy serwowany element
} job;

// Algorytmy przetwarzania (uzwanye przez getopts parser)
enum {
  SJF = 0,
  SRTF,
  THE_END
};
// Algorytmy przetwarzania (uzwanye przez getopts parser)
char *scheduler_opts[] = {
  [SJF]     = "sjf",
  [SRTF]    = "srtf",
  [THE_END] = NULL

};

//zmienne globalne
int sum_context = 0;

// zasada porowniania
//     za kazdym razem wez dwa wskazniki do obiektu i porownaj je, zwracajac
//     0 jezeli sa rowne, falsz jezeli a < b, albo prawde
//     jezeli a > b
int fcfs_comparison(void *a, void *b);
int sjf_comparison(void *a, void *b);
int srtf_comparison(void *a, void *b);
int rr_comparison(void *a, void *b);
int id_comparison(void *a, void *b);

// przetwarzanie
void build_job(job *j, int id, int arrive, int burst);
void increment_waits(heap *h);
void generate_jobs(heap *h, int (*comp_func)(void*, void*), int number_of_jobs);
void read_jobs_from_file(heap *h, int (*comp_func)(void*, void*), char *filename);
void process_jobs(int (*comp_func)(void*, void*), char *filename, int n, int verbose);

// wyswietlanie
void print_title(int (*comp_func)(void*, void*));
void print_job(job *j);
void print_results(heap *c, int verbose);
void print_usage(int argc, char *argv[]);

// porownywanie oparte na czasie pojawienia sie, potem id 
int fcfs_comparison(void *a, void *b)
{
    int retval;
    retval = (((job*)b)->arrive - ((job*)a)->arrive);
    if(retval == 0) retval = (((job*)b)->id - ((job*)a)->id);
    return retval;
}

// porownainie oparte na czasie pozostalym do przetworzenia, potem id
int sjf_comparison(void *a, void *b)
{
    int retval;
    retval = (((job*)b)->burst - ((job*)a)->burst);
    if(retval == 0) retval = (((job*)b)->id - ((job*)a)->id);
    return retval;
}

// porownanie oparte na pozostalym czasie przetworzenia (lecz z wywlaszczeniem), potem wg. id
int srtf_comparison(void *a, void *b)
{
    int retval;
    retval = (((job*)b)->burst - ((job*)a)->burst);
    if(retval == 0) retval = (((job*)b)->id - ((job*)a)->id);
    return retval;
}

// porownanie oparte na priorytetach, potem na id
int rr_comparison(void *a, void *b)
{
    int retval;
    retval = (((job*)b)->priority - ((job*)a)->priority);
    if(retval == 0) retval = (((job*)b)->arrive - ((job*)a)->arrive);
    if(retval == 0) retval = (((job*)b)->id - ((job*)a)->id);
    return retval;
}

// porownanie oparte na priorytecie, potem na id
int unix_comparison(void *a, void *b)
{
    int retval;
    retval = (((job*)b)->priority - ((job*)a)->priority);
    if(retval == 0) retval = (((job*)b)->arrive - ((job*)a)->arrive);
    if(retval == 0) retval = (((job*)b)->id - ((job*)a)->id);
    return retval;
}

// porownanie oparte na id
int id_comparison(void *a, void *b)
{
    return (((job*)b)->id - ((job*)a)->id);
}

// tworzenie zadan w oparciu o dane
void build_job(job *j, int id, int arrive, int burst)
{
    j->id       = id;
    j->arrive   = arrive;
    j->burst    = burst;
    j->priority = 0;
    j->waiting  = 0;
    j->start    = -1;
    j->end      = 0;
    j->service  = 0;
    return;
}

// iterowanie przez wszystkie zadanaia i zwiekszenie ich sumarycznego czasu oczekiwania
void increment_waits(heap *h)
{
    int i;
    for(i = 1; i <= h->size; i++)
        ((job*)(h->a[i]))->waiting++;
    return;
}

// iteruj przez wszystkie zadania i przelicz priorytety
void recalculate_priorities(heap *h, int current_time)
{
    int i;
    job *j = NULL;
    for(i = 1; i <= h->size; i++) {
        j = (job*)(h->a[i]);
        j->priority = (j->service)/2 + 60;
    }
    return;
}

// utworz losowe zadania i wstaw je na stos
void generate_jobs(heap *h, int (*comp_func)(void*, void*), int number_of_jobs)
{
    int i,
        arrive,
        burst;

    for(srand(time(NULL)), i = 1, arrive = 0, h->size = 0; // initialize variables
        i <= number_of_jobs; // loop until i = the number_of_jobs specified
        i++, arrive += (rand() % 7)) // we increment "arrive" by a random amount
    {
        burst = 2 + (rand() % 5); // set burst to random number
        job *temp = malloc(sizeof(job)); // create a job
        build_job(temp, i, arrive, burst); // populate job
        heap_insert(h, comp_func, temp); // stick it in the queue
    }
    return;
}

// odczytaj zadania z pliku CSV. format pliku "arrive,burst,priority" - jeden rekord na linie.
void read_jobs_from_file(heap *h, int (*comp_func)(void*, void*), char *filename)
{
    char buffer[256];
    char *temp;
    int i           = 0,
        arrive      = 0,
        burst       = 0;
    FILE *fp        = NULL;
    if((fp = fopen(filename, "r"))) {
        while (!feof(fp)) {
            fgets(buffer, 256, fp); // read a line

            if(strlen(buffer) > 1) {        
                if((temp = strtok(buffer, ",\n")) != NULL && strlen(temp) > 0)
                    arrive += strtol(temp, NULL, 10);
                if((temp = strtok(NULL, ",\n")) != NULL && strlen(temp) > 0)
                    burst = strtol(temp, NULL, 10);
                job *temp = malloc(sizeof(job)); // create a new job
                build_job(temp, i++, arrive, burst); // populate it
                heap_insert(h, comp_func, temp); // stick it in the queue
            }
        }
        fclose(fp);
    }
    else {
        fprintf(stderr, "error opening file %s\n", filename);
        exit(1);
    }
    return;
}

// serce programu
// comp_func    - funkcja uzyta do pozyskania priorytetu 
// filename     - plik z danmi wejsciowymi 
// n            - liczba procesow do wygenerowania (jezeli nie uzywamy danych z pliku)
// verbose      - jezeli prawda, zwroci informacje odnosnie kazdego 
void process_jobs(int (*comp_func)(void*, void*), char *filename, int n, int verbose)
{
    int i; // glowna wartosc licznika
    int done = FALSE;
    job *current = NULL; // aktualnie przetwarzane zadanie

    heap *g = malloc(sizeof(heap)); // tworzy kolejke - trzyma zadania wygenerowane albo przeczytane z pliku
    heap *p = malloc(sizeof(heap)); // kolejka procesow - trzyma przetwarzane zadania
    heap *c = malloc(sizeof(heap)); // kolejka przetworzonych procesow - trzyma zadania ktore zostaly juz przetworzone
    heap_init(g);
    heap_init(p);
    heap_init(c);

    if(filename) // jezeli mamy plik  to go przeczytajmy 
        read_jobs_from_file(g, fcfs_comparison, filename);
    else // w innym wypadku sami generujemy dane
        generate_jobs(g, fcfs_comparison, n);

    print_title(comp_func);
    
    for(i = 0; !done; i++)
    {
        // pobierz nastepny "nadchodzacy" proces z kolejki i 
				// wstaw na kolejke do przetwarzania
        job *insert, *temp;
        while((insert = heap_extract_max(g, fcfs_comparison)) && insert->arrive <= i) {
            if(comp_func == &rr_comparison) insert->priority = i;
            heap_insert(p, comp_func, insert);
            if(comp_func == &unix_comparison) recalculate_priorities(p, i);                     
            insert = NULL;
        }
        if(insert && insert->arrive > i) // mozemy wstawic za duzo elementow 
            heap_insert(g, fcfs_comparison, insert); // wiec wstawmy je spowrotem

        if(current == NULL) {
            temp = current;
            current = heap_extract_max(p, comp_func);
            if(temp != current) {
                if(verbose) printf("zegar: %2d\tzmiana kontekstu\n", i++);
                sum_context++;
                increment_waits(p);
            }
        }
        else {
            if(comp_func == &srtf_comparison
                || comp_func == &rr_comparison
                || comp_func == &unix_comparison)
            {
                temp = current;
                if(comp_func == &rr_comparison) current->priority = i;
                heap_insert(p, comp_func, current);
                if(comp_func == &unix_comparison) recalculate_priorities(p, i);
                current = heap_extract_max(p, comp_func);
                if(temp != current) {
                    if(verbose) printf("zegar: %2d\tzmiana kontekstu\n", i++);
                    sum_context++;
                    increment_waits(p);
                }
            }
        }
        if(current == NULL) {
            if(g->size == 0) done = TRUE;
            else if(verbose) printf("zegar: %2d\tidle\n", i);
        }
        else {
            if(current->arrive <= i) {
                if(current->start < 0) current->start = i; // zaznacz start jezeli pierwszy raz na aktywnej kolejce CPU
                current->service++; // zwieksz czas obslugi 
                current->burst--; // zmniejsz pozostaly czas do zakonczenia 
                if(verbose) {
                    printf("zegar: %2d\t", i);
                    print_job(current);
                }

                // jezeli zadanie zostalo zakonczone, przesun je na skonczona kolejke
                if(current->burst <= 0) {
                    current->end = i; // zaznacz czas zakonczenia dla ukonczonego zadania 
                    heap_insert(c, id_comparison, current); // wstaw zadanie na kolejke zadan przetworzonych
                    current = NULL;
                }
                increment_waits(p); // zwieksz sumaryczna wartosc czekania dla wszystkich zadan ktore oczekuja
            }
            else if(verbose) printf("zegar: %2d\tidle\n", i);
        }
    }
    print_results(c, verbose); // wypisz podsumowanie i analize przeprowadzonych obliczen
    return;
}

// wyswietl nazwe wybranego algorytmu
void print_title(int (*comp_func)(void*, void*))
{
    char *algorithm_name = NULL;
    if(comp_func == &sjf_comparison) {
        algorithm_name = "Shortest Job First";
    } else if(comp_func == &fcfs_comparison) {
        algorithm_name = "First Come First Served";
    } else if(comp_func == &srtf_comparison) {
        algorithm_name = "Shortest Remaining Time First";
    } else if(comp_func == &rr_comparison) {
        algorithm_name = "Round Robin";
    } else if(comp_func == &unix_comparison) {
        algorithm_name = "POSIX Dynamic Priorities";
    } else
        algorithm_name = "Unknown";
    printf("\n*** %s ***\n", algorithm_name);
    return;
}

// wyswietl przetwarzane zadanie
void print_job(job *j)
{
    printf("id: %3d\tprzybycie: %4d\tburst: %4d\tczekanie: %4d\tstart: %4d\tkoniec: %4d\tpriorytet: %4d\tobsluga: %4d\n",
        j->id,
        j->arrive,
        j->burst,
        j->waiting,
        j->start,
        j->end,
        j->priority,
        j->service);
    return;
}

// wyswietl podsumowanie i analize
void print_results(heap *c, int verbose)
{
    job *current       = NULL;
    int number_of_jobs = c->size,
        sum_service    = 0,
        sum_waiting    = 0,
        sum_turnaround = 0,
        sum_response   = 0,
        max_end        = 0;
    if(verbose) printf("final przetwarzania zadania:\n");
    while((current = heap_extract_max(c, id_comparison))) {
        sum_service += current->service;
        sum_waiting += current->waiting;
        sum_turnaround += (current->end - current->arrive);
        sum_response += (current->start - current->arrive);
        max_end = max_end > current->end ? max_end : current->end;
        if(verbose) print_job(current);
    }
    printf("podsumowanie:\n");
    printf(" razem zadan:\t\t%d zadan\n", number_of_jobs);
    printf(" przepustowosc:\t\t\t%3.2f zadan/ms\n", (float)number_of_jobs / (float)max_end);
    printf(" wykorzystanie cpu:\t\t\t%3.2f%%\n", ((float)sum_service/((float)sum_context + (float)sum_service)) * 100.0);
    printf(" czas przetworzenia:\t%3.2f ms\n", (float)sum_turnaround / (float)number_of_jobs);
    printf(" czas odpowiedzi:\t\t%3.2f ms\n", (float)sum_response / (float)number_of_jobs);
    printf(" czas oczekiwania:\t\t%3.2f ms\n", (float)sum_waiting / (float)number_of_jobs);
    return;
}

// zwroc pomoc, opcje uzycia 
void print_usage(int argc, char *argv[])
{
    printf("uzycie:\t\t%s [OPCJE]\n", argv[0]);
    printf("przyklad:\t%s -i data.txt -s srtf -v\n", argv[0]);
    printf("\t\t%s -n 5 -s srtf\n", argv[0]);
    printf("opcje:\n");
    printf(" -h\t\tPokazuje ta wiadomosc.\n");
    printf(" -i <plik>\tOczytuje plik oddzielony przecinkami CZAS_PRZYBYCIA,CZAS_DZIALANIA\n");
    printf(" -n <liczba>\tIlosc zdan do wykonania jezeli plik nie jest podany.\n");
    printf(" -s <algorytm>\tUzywny algorytm.\n");
    printf(" \t\tZaimplementowane algorytmy: sjf, srtf\n");
    printf(" -v\t\tTryb debugowania. Wyswietla kazdy cykl procesora.\n");
    return;
}
