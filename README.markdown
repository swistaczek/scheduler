Ten projekt został zrealizowany na zajęcia z Systemów Operacyjnych prowadzonych przez dr. Jerzego Bartoszka na Politechnice Poznańskiej. Realizuję powierzony problem działania kolejkowania procesów procesora - SRTF jednocześnie dając platformę do implementacji innych mechanizmów kolejkowania procesów. 

---

# Proces kompilacji i użytkowanie
Proszę uruchom "make clean && make" aby skompilować program.
Program można uruchomić skryptem (runall.sh) bądź manualnie (./a2) jak opisano niżej:

    uzycie:		./a2 [OPCJE]
    przyklad:	./a2 -i data.txt -s srtf -v
    		./a2 -n 5 -s srtf
    opcje:
     -h		Pokazuje ta wiadomosc.
     -i <plik>	Oczytuje plik oddzielony przecinkami CZAS_PRZYBYCIA,CZAS_DZIALANIA
     -n <liczba>	Ilosc zdan do wykonania jezeli plik nie jest podany.
     -s <algorytm>	Uzywny algorytm.
     		Zaimplementowane algorytmy: sjf, srtf
     -v		Tryb debugowania. Wyswietla kazdy cykl procesora.
    musisz podac -n albo -i.
    uzycie:		./a2 [OPCJE]
    przyklad:	./a2 -i data.txt -s srtf -v
    		./a2 -n 5 -s srtf
    opcje:
     -h		Pokazuje ta wiadomosc.
     -i <plik>	Oczytuje plik oddzielony przecinkami CZAS_PRZYBYCIA,CZAS_DZIALANIA
     -n <liczba>	Ilosc zdan do wykonania jezeli plik nie jest podany.
     -s <algorytm>	Uzywny algorytm.
     		Zaimplementowane algorytmy: sjf, srtf
     -v		Tryb debugowania. Wyswietla kazdy cykl procesora.

---

# Systemy operacyjne
# Algorytmy czasu procesora
#### Ernest Bursa
#### Politechnika Poznańska
#### ernest@bzdury.pl

## Wstęp
Zadaniem programu jest zademonostrowanie działania algorytmów przydziału czasu procesora. Dane wprowadza się manualnie bądź zleca się ich generacje maszynie pseudolosowej. 
## 1 Wprowadzenie
Kolejkowanie procesów procesora jest procesem który determinuje jak, w jakiej kolejności i jak długo system operacyjny będzie przetwarzał dane zadanie. Wprowadzone wartości takie jak algorytm kolejkowania, długość procesu i częstotliwość powtarzania kolejki są dopuszczalne przy użyciu procesora. Na przykład, system który jest zaprojektowany do przetwarzania cięższych zadań we współpracy z użytkownikiem może wymagać mniejszego średniego czasu odpowiedzi na zlecenie (proces) tak aby użytkownik nie odczuł dyskomfortu związanego z opóźnieniem przetwarzania procesu. 

Na potrzeby zadania zaimplementowałem dwa algorytmy przydziału czasu procesora:
* First Come First Served
* Shortest Remaining Time First

Można obserwować takie parametry jak:
* Wydajność zadania
* Zużycie CPU - CPUUT
* Średni czas przetworzenia
* Średni czas odpowiedzi
* Średni czas oczekiwania

## 1.1 Algorytmy kolejkowania i zarządzania czasem procesora
## 1.1.1 First Come First Served (FCFS)
Najprostszy, niewywłaszczający algorytm planowania dostępu do procesora: przydział procesora następuje w kolejności “pierwszy nadszedł – pierwszy obsłużony”. Według tego schematu proces, który pierwszy zamówi procesor, pierwszy go otrzyma. Algorytm FCFS implementuje się za pomocą kolejki FIFO: blok kontrolny procesu wchodzącego do kolejki jest dołączany na jej końcu. Wolny procesor przydziela się procesowi z czoła kolejki. Algorytm planowania metodą FCFS może powodować efekt konwoju wskutek długotrwałego zajmowania procesora przez niektóre procesy, co niekorzystnie wpływa na średni czas oczekiwania procesów w kolejce do procesora

## 1.1.3 Shortest Remaining Time First (SRTF)
"Najpierw zadanie o najkrótszym pozostałym czasie wykonania" to wersja strategii SJF z wywłaszczaniem. W strategii tej decyzja planisty jest oczywiście podejmowana w chwili zakończenia fazy procesora procesu aktywnego, ale również w chwili, gdy którykolwiek proces zmienia stan na gotowy. W takiej chwili wybiera się proces, którego pozostały przewidywany czas fazy procesora jest najkrótszy. Jeśli w trakcie wykonywania procesu pojawiły się inny gotowy proces, którego oczekiwany czas fazy procesora jest krótszy niż pozostały oczekiwany czas działania procesu aktywnego, to proces aktywny jest wywłaszczany a w jego miejsce procesor otrzymuje nowo przybyły proces. 


## 1.2 Szczegóły parametrów
## 1.2.1 Wydajność zadania
Jest to ilość zadań dla procesora w czasie określonej jednostki czasu, mierzone w milisekundach.

## 1.2.2 Zużycie CPU - CPUUT
Wartość podawania w procentach, reprezentuje ilość czasu jaki procesor poświęcił na przetwarzanie zadań w stosunku do czasu jaki poświęcił na przełączanie. Pzełączanie pojawia się np. w momencie kiedy proces jest przesuwany na kolejkę boczną. System musi wstrzymać aktualne zadanie, zachować przetwarzany kontekst i środowisko i przesunąć je w bezpieczne miejsce (np. pamięć) tak by móc później bezpiecznie wznowić proces. Minimalizacja czasu przełączania kontekstów jest głównym punktem wg. którego algorytm przydziału czasu procesora winien być zoptymalizowany.

## 1.2.3 Średni czas przetworzenia 
Jednostka czasu jakie zajmuje zaserwowanie procesu od momentu 'dostarczenia' do momentu ukończenia.

## 1.2.4 Średni czas odpowiedzi
Jednostka czasu od momentu kiedy proces zostaje 'dostarczony' do momentu pojawienia sie pierwszej odpowiedzi od procesu. 

## 1.2.5 Średni czas oczekiwania
Jednostka czasu sumarycznie poświęconego na czekanie, włącznie z czasem poświęconym na pierwszy cykl a także czas poświęcony na przełączanie pomiedzy procesami o wyższym priorytecie.

## 2 Program
## 2.1 Struktury danych i algorytmy
## 2.2.1 Stos (Heap)
Sercem "schedulera" jest klasa heap. Stos to bardzo prosta struktura danych która jest zaimplementowana jako tablica ale z kilkoma specjalnymi zasadami które pozwalają nam bardzo szybko i łatwo wydobyć element z najwyższym priorytetem. Można rzec że jest to kolejka priorytetów. Umieszczanie i pobieranie danych mają złożoność obliczeniową na poziomie O(lg n). Znakomicie nadaje się do zaimplementowania kolejki procesów.

Kolejka priorytetów, niezależnie od implemetacji ma także ogólne zastosowanie dla tego zadania ze względu na łatwą możliwość adaptacji do nowych sytuacji. Przesuwając ciężar porównań na 'trzeci' (funkcje porównujące) moduł możemy w łatwy sposób zmienić typ wykorzystywanego algorytmu kolejkowania czasu procesora. Dla przykładu kolejka w algorytmie FCFS będzie sortować proecsy po czasie przybycia gdzie inny algorytm (np. SJF) będzie sortował po czasie dlugości przybycia procesora.

## 2.2.2 Funkcje porównujące
Jak opisałem we wcześniejszej sekcji, kolejka priorytetów jest zaimplementowana jako stos, pozwalając ponownie wykorzystać kod w treści programu.
Aby tego dokonać stworzyłem wszystkie metody dla stosu tak by wymagały wywołania funkcji porównującej. Te reguły wykorzystują następnie podaną przez użytkownika funckcje porównania tak by wyliczyć priorytet poszczególnych elementów na stosie (posortować go). Zasada działania sortowania jest ogólnie zbliżona do działania qsort (ze standardowej biblioteki C stdlib.h). Funkcja porównania otrzymuje dwa argumenty - a i b. Jeżeli a < b to funkcja zwraca wartość fałsz (false). Jeżeli a > b to funkcja zwraca prawdę (true). Jeżeli a jest równe b to funkcja zwraca zero. Zasada ta zrealizowana jest poprzez odejmowanie b od a.

## 2.2.3 Struktura zadania
Moja struktura zadania to prosta struktura C z następującymi własnościami:
* **id** Id zadania
* **arrive** Czas przybycia
* **burst** Pozostały czas działania
* **waiting** Suma czasu oczekiwania
* **start** Czas rozpiczęcia. Domyślnie ustawiony na -1 do momentu utworzenia obiektu.
* **end** Czas zakończenia
* **priority** Priorytet zadania. Może być wykorzystany do późniejszej implemetancji np. Round Robin. 
* **service** Sumaryczny czas przetwarzaniaThe total service time

## 2.2.4 Wywłaszczenie
Algorytm SRTF jest algorytmem z wywłaszczeniem. Wywłaszczenie zostało zaimplementowane przez "ponowne uruchomienie" aktualnie przetwarzanego zadania na koniec stosu po każdym "tyknięciu" zegara procesora w celu ponownego wykonania całego stosu tak by sprawdzić czy inne zadanie z wyższym priorytetem nie powinno zająć miejsca aktualnie przetwarzanego zadania.

## 2.2.5 Generowanie danych
Aby symulować działanie prawdziwych systemów potrzebujemy generator danych losowych. Do tego celu wykorzystałem program na licencji GNU - Octave. Program z otwartym źródłem podobny dl Wolfram MATLAB'a. Octave posiada funkcję exprnd która akceptuje wartości lambda. Z jej pomoca generuje zawartość tabeli, stworzyłem macierz z 500 elementami i dwiema kolumnami. Pierwsza kolumna reprezentuje czas przybycia zadania procesu do procesora, natomiast druga kolumna symbolizuje szacowany czas przetwarzania. Octave zezwala na eksport danych do formatu CSV (dane oddzielone przecinkami i oddzielone znakiem nowej linni).

## 3. Wynik
Badania ekspertów pokazują że gdy responsywność jest najważniejszą cechą systemu (np. system przeznaczony do bezpośredniego kontaktu z użytkownikiem) kolejkowanie POSIX powinno zapewnić maksimum wydajnośći wg. tego priorytetu, natomiast FCFS zupełnie nie nadaje się do tego typu zadań. Algorytm Round Robin ma okropną średnią czasów oczekiwania w porównaniu do innych algorytmów. Algorytmy SJF i SRTF dają sobie radę w większości przypadków. Tak czy inaczej wybór konkretnego algorytmu powinien być dopasowany do specyfiki obsługiwanych aplikacji. 

## 4. Bibliografia
[1] Weisstein, Eric W. "Exponential Distribution." From MathWorld--A Wolfram Web Resource. [http://mathworld.wolfram.com/ExponentialDistribution.html](http://mathworld.wolfram.com/ExponentialDistribution.html)
[2] Wikipedia [http://en.wikipedia.org/wiki/Shortest_remaining_time](http://en.wikipedia.org/wiki/Shortest_remaining_time)
[3] Materiały uniwersytetu Stanford [http://www.scs.stanford.edu/07au-cs140/notes/l5.pdf](http://www.scs.stanford.edu/07au-cs140/notes/l5.pdf)