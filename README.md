# ComputerNetworks_WisielecGame

Opis:
Gracz łączy się do serwera i wysyła swój nick (jeśli nick jest już zajęty, serwer prosi o podanie innego nicku). Jeśli użytkownik jest administratorem jako nick podaje „admin” i następnie musi wpisać hasło.
Administrator ma możliwość podglądu obecnie prowadzonych gier, ilości graczy. Ma też opcję zakończenia obecnie trwającej gry.
Gracz po wybraniu nicku trafia do lobby, w którym widzi bieżącą listę oczekujących na rozpoczęcie gier.
Z lobby gracz może dołączyć do pokoju nierozpoczętej gry. Gracz ma także opcję stworzenia własnego pokoju, staje się wtedy jego założycielem. Gracz może w każdej chwili wrócić z pokoju do lobby.
Po dołączeniu do pokoju gracz widzi listę graczy oczekujących w pokoju i czeka na rozpoczęcie gry. Gracz w każdym momencie gry może opuścić pokój i wrócić do lobby.
Grę może rozpocząć jej założyciel, pod warunkiem, że w pokoju jest minimum dwóch graczy.
Graczom pokazywane jest hasło z literami zastąpionymi przez kreski oraz klawiatura z literami do wyboru. Gracze przez cały czas również widzą okna wisielcy swojego i innych graczy.  
Gracz zgaduje litery. Jeśli hasło zawiera literę, którą wybrał gracz, dostaje on punkt, a każdemu graczowi odsłaniają się wszystkie jej powtórzenia w haśle. Każdemu graczowi usuwa się opcja wyboru tej litery. Jeśli litery nie ma w haśle dorysowywana jest kolejna część wisielca i usuwa się opcja wyboru tej litery dla tego gracza.
Jeśli kilku graczy wybrało tą samą literę przed ujawnieniem jej w haśle, to każdy dostaje punkt.
Jeśli wisielec danego gracza zostanie skompletowany, gracz staje się nieaktywny i traci możliwość dalszego zgadywania haseł.
Gra kończy się, gdy zostanie tylko jeden aktywny gracz. Po zakończeniu gry wyświetlają się wyniki. Wygrywa gracz, który zdobył największą liczbę punktów, niezależnie od tego, kiedy „zawisł”.  Po wyświetleniu wyniku gracz wraca do lobby
