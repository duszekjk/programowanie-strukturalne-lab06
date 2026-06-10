# Unreal IPC Lab Guide

Ten plik opisuje, jak korzystać z gotowego projektu Unreal Engine w tym labie.

Założenie jest proste:

- uczniowie/studenti nie piszą kodu w Unreal,
- piszą małe programy w C, które wysyłają tekstowe komendy do FIFO,
- gra Unreal odbiera komendy i steruje postacią,
- po zebraniu diamentu może zostać wysłany `final_answer` na serwer zajęciowy,
- wysyłka jest dozwolona tylko wtedy, gdy cała runda była sterowana programowo, bez użycia klawiatury.

## 1. Co jest w projekcie

Najważniejsze elementy:

- gotowy projekt Unreal Engine 5.7 bazujący na Third Person Template,
- komponent IPC czytający komendy z FIFO `/tmp/ue_character_commands`,
- 10 diamentów/cube'ów rozmieszczonych w scenie,
- dwa liczniki postępu:
  - zebrane automatycznie,
  - zebrane ręcznie,
- przykładowe programy w C w katalogu `examples/`,
- helper `include/ue_ipc.h` do wysyłania pojedynczych komend,
- plik konfiguracyjny JSON dla danych wysyłki do serwera.

## 2. Jak działa komunikacja

Komunikacja odbywa się przez FIFO, czyli named pipe.

Domyślna ścieżka:

```text
/tmp/ue_character_commands
```

Każda komenda to jedna linia tekstu zakończona znakiem nowej linii:

```text
FORWARD
RIGHT
JUMP
RESET
PING
```

W projekcie obsługiwane są:

- `FORWARD`
- `BACKWARD`
- `LEFT`
- `RIGHT`
- `JUMP`
- `STOP`
- `RESET`
- `PING`

Komendy są niewrażliwe na wielkość liter. `forward`, `Forward` i `FORWARD` działają tak samo.

## 3. Zasada auto/manual

Projekt rozróżnia dwa sposoby sterowania:

- automatyczny, przez IPC,
- ręczny, przez klawiaturę.

Jeżeli w danej rundzie użyjesz klawiatury choć raz, to ta runda jest oznaczana jako ręczna.

To ma znaczenie dla wysyłki:

- diament zebrany wyłącznie przez IPC może uruchomić wysyłanie `final_answer`,
- diament zebrany po użyciu klawiatury nie wyśle odpowiedzi na serwer.

W praktyce:

- jeśli chcesz uzyskać pełną automatyzację, nie dotykaj klawiatury podczas gry,
- po użyciu klawiatury nie licz już na automatyczną wysyłkę w tej samej sesji,
- restart gry resetuje stan rundy, ale zwykły `RESET` pozycji nie zmienia faktu, że klawiatura była użyta.

## 4. Programy sterujące w C

Najprostszy program wygląda tak:

```c
#include "../include/ue_ipc.h"

int main(void)
{
    return send_command("FORWARD");
}
```

Helper `ue_ipc.h`:

- otwiera FIFO `/tmp/ue_character_commands`,
- zapisuje komendę,
- dopisuje znak nowej linii,
- zamyka deskryptor.

### Przykład ruchu po kwadracie

`examples/square.c` pokazuje klasyczny wzorzec:

```c
for (int i = 0; i < 4; ++i)
{
    send_command("FORWARD");
    send_command("RIGHT");
}
```

To jest dobry punkt startowy dla własnych programów:

- jedna komenda = jeden impuls,
- jeśli chcesz dłuższy ruch, wysyłasz komendę kilka razy,
- nie próbujesz wysyłać parametrów typu `FORWARD 2`.

### Jak pisać własne programy

Najprostszy schemat:

1. dołącz `ue_ipc.h`,
2. w `main()` wywołuj `send_command(...)`,
3. sprawdzaj wynik funkcji,
4. jeśli chcesz zbudować trasę, użyj pętli,
5. jeśli chcesz reagować na klawisze w terminalu, zrób to po stronie programu C, a nie w Unreal.

Przykład z obsługą błędu:

```c
#include "../include/ue_ipc.h"

int main(void)
{
    if (send_command("FORWARD") != 0)
    {
        return 1;
    }

    if (send_command("RIGHT") != 0)
    {
        return 1;
    }

    return 0;
}
```

## 5. Jak zbudować programy C na Ubuntu

Na Ubuntu potrzebujesz standardowego narzędzia do kompilacji.

Instalacja:

```bash
sudo apt update
sudo apt install build-essential
```

Budowa wszystkich przykładów:

```bash
make
```

Pojedynczy program można zbudować ręcznie:

```bash
gcc -std=c11 -Wall -Wextra -pedantic -O2 examples/forward.c -o forward
```

Uruchomienie:

```bash
./forward
```

Jeżeli Unreal działa w tym samym czasie i FIFO istnieje, postać dostanie komendę.

## 6. Jak uruchomić projekt Unreal

### Wariant zalecany na zajęciach

1. Zbuduj albo spakuj grę jako projekt Unreal Engine 5.7.
2. Upewnij się, że wynikowy folder zawiera skrypt uruchomieniowy `Game/IpcCharacterWorld.sh`.
3. Skopiuj paczkę na Ubuntu.
4. Na Ubuntu uruchom:

```bash
./Game/IpcCharacterWorld.sh
```

5. W drugim terminalu uruchom program C:

```bash
./square
```

albo wyślij pojedynczą komendę:

```bash
echo FORWARD > /tmp/ue_character_commands
```

### Budowa gry z Windows na Linux

Jeżeli projekt jest rozwijany na Windows, a ma działać na Ubuntu:

1. Otwórz projekt w Unreal Editor.
2. Użyj pakowania na platformę Linux.
3. Skopiuj gotową paczkę na Ubuntu.
4. Uruchom ją tam, gdzie docelowo ma działać lab.

Jeżeli nie masz skonfigurowanego pakowania na Linux z Windows, najprościej:

- zrobić build projektu na maszynie Linux,
- albo użyć gotowej paczki przygotowanej przez prowadzącego.

## 7. Jak skonfigurować wysyłkę do serwera

W projekcie można użyć pliku JSON z danymi studenta.

Plik:

```text
Config/SubmissionConfig.json
```

Przykładowe pola:

```json
{
  "base_url": "https://www.duszekjk.com/programowaniestrukturalne/api/submit_answer/",
  "student_index": "123456",
  "student_mail": "jan.kowalski@example.com",
  "first_name": "Jan",
  "last_name": "Kowalski",
  "group_number": "3",
  "share_link": "https://example.com/share"
}
```

Znaczenie pól:

- `student_index` - numer indeksu,
- `group_number` - numer grupy,
- `student_mail` - adres e-mail,
- `first_name` - imię, opcjonalne,
- `last_name` - nazwisko, opcjonalne,
- `share_link` - dowolny link pomocniczy, jeśli potrzebny,
- `base_url` - adres serwera z odpowiedziami.

Jeżeli plik nie istnieje, projekt użyje domyślnych wartości z kodu.

## 8. Format wiadomości wysyłanej na serwer

Po zebraniu diamentu automatycznie projekt buduje JSON z polami:

- `student_id`
- `student_mail`
- `first_name`
- `last_name`
- `task`
- `grupa`
- `answer`
- `share_link`

Ważne:

- `student_id` jest w praktyce numerem indeksu,
- `grupa` jest numerem grupy,
- `answer` zawiera kod diamentu,
- kod jest skracany zgodnie z regułą zajęć i dopisywany jest rozmiar odpowiedzi.

## 9. Jak testować komunikację

### Test minimalny

1. Uruchom grę.
2. W drugim terminalu wyślij:

```bash
echo PING > /tmp/ue_character_commands
```

3. Sprawdź, czy w logu gry pojawił się ping.

### Test ruchu

```bash
echo FORWARD > /tmp/ue_character_commands
echo RIGHT > /tmp/ue_character_commands
echo FORWARD > /tmp/ue_character_commands
```

### Test programu C

```bash
./forward
./jump
./square
```

### Test błędnej komendy

```bash
echo FLY > /tmp/ue_character_commands
```

Gra powinna to zignorować i zalogować jako komendę niepoprawną.

## 10. Jak rozpoznać, że wszystko działa

Powinieneś widzieć:

- postać z Third Person Template,
- 10 diamentów na mapie,
- dwa liczniki na ekranie debug,
- reakcję na komendy z FIFO,
- brak crasha po błędnej komendzie,
- wysyłkę do serwera tylko wtedy, gdy nie użyto klawiatury.

## 11. Najczęstsze problemy

### Gra nie reaguje na komendy

Sprawdź:

- czy gra działa,
- czy FIFO `/tmp/ue_character_commands` istnieje,
- czy wysyłasz komendy jako zwykły tekst zakończony nową linią,
- czy uruchamiasz program po starcie gry.

### Program C nie kompiluje się

Sprawdź:

- czy masz `gcc` albo `clang`,
- czy kompilujesz na Linuxie,
- czy używasz `make` z tego repo,
- czy nie zmieniłeś ścieżki do `include/ue_ipc.h`.

### Diament się zbiera, ale nie ma wysyłki

Najczęstsze powody:

- użyto klawiatury w tej samej rundzie,
- nie ma poprawnego `SubmissionConfig.json`,
- `base_url` jest pusty,
- serwer nie odpowiada.

## 12. Dobre praktyki dla studentów

- Najpierw testuj pojedyncze komendy.
- Potem buduj krótkie sekwencje.
- Na końcu pisz większy program w C.
- Jeśli chcesz automatyczną wysyłkę, steruj tylko IPC.
- Jeżeli potrzebujesz restartu sesji, zamknij i uruchom grę ponownie.

