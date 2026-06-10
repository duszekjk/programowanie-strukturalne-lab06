# Specyfikacja dla Codexa: Unreal IPC Character Controller

## Cel

Przygotować gotowy projekt demonstracyjny do zajęć z oprogramowania systemowego / komunikacji międzyprocesowej.

Studenci **nie programują w Unreal Engine**. Ich zadaniem jest pisanie prostych programów w C, które wysyłają komendy sterujące postacią w gotowej aplikacji Unreal Engine.

Docelowy efekt:

1. Prowadzący uruchamia gotową aplikację Unreal Engine.
2. Student kompiluje swój program w C.
3. Program studenta wysyła komendy przez IPC.
4. Postać w scenie Unreal Engine wykonuje ruchy.

Przykład:

```bash
./Game/IpcCharacterWorld.sh
gcc examples/square.c -o square
./square
```

Postać w grze idzie do przodu, skręca, idzie dalej itd.

---

## Założenia dydaktyczne

To ćwiczenie ma pokazywać komunikację między procesami, a nie programowanie w Unreal Engine.

Dlatego:

- Unreal Engine ma być przygotowany wcześniej.
- Studenci mają dostać gotową paczkę z aplikacją gry.
- Studenci piszą tylko programy w C.
- Komendy są proste tekstowo.
- Jedna komenda oznacza jeden krótki krok / impuls.
- Nie używamy parametrów typu `FORWARD 2.0`.
- Jeśli student chce wykonać ruch dwa razy, wysyła komendę dwa razy.

Przykład:

```text
FORWARD
FORWARD
RIGHT
FORWARD
JUMP
```

---

## Platforma docelowa

Podstawowa platforma: **Linux**, najlepiej Ubuntu 24.04 LTS albo zbliżona dystrybucja.

Powód: tematem zajęć jest oprogramowanie systemowe i IPC, a Linux daje najprostsze środowisko do użycia FIFO / named pipe.

Wariant opcjonalny: macOS.  
Wariant Windows nie jest wymagany w pierwszej wersji.

---

## Mechanizm IPC

Użyć **FIFO / named pipe**.

Ścieżka domyślna:

```text
/tmp/ue_character_commands
```

Unreal Engine czyta z tej kolejki komendy tekstowe.

Programy studentów piszą do tej kolejki.

Test z terminala ma działać:

```bash
echo FORWARD > /tmp/ue_character_commands
echo RIGHT > /tmp/ue_character_commands
echo FORWARD > /tmp/ue_character_commands
```

---

## Protokół komend

Każda komenda to jedna linia tekstu zakończona `\n`.

Obsługiwane komendy w pierwszej wersji:

```text
FORWARD
BACKWARD
LEFT
RIGHT
JUMP
STOP
RESET
PING
```

Znaczenie:

| Komenda | Efekt |
|---|---|
| `FORWARD` | postać wykonuje jeden krótki ruch do przodu |
| `BACKWARD` | postać wykonuje jeden krótki ruch do tyłu |
| `LEFT` | postać skręca w lewo o ustalony kąt albo przez krótki impuls |
| `RIGHT` | postać skręca w prawo o ustalony kąt albo przez krótki impuls |
| `JUMP` | postać skacze |
| `STOP` | wyzerowanie aktualnego ruchu |
| `RESET` | powrót postaci do pozycji startowej |
| `PING` | komenda testowa, wypisuje log w Unreal, bez ruchu |

Komendy mają być case-insensitive, czyli `forward`, `Forward` i `FORWARD` mają działać tak samo.

Błędna komenda nie powinna crashować gry. Ma zostać zignorowana i zapisana w logu.

---

## Semantyka ruchu

Ponieważ komendy nie mają parametrów, każda komenda ruchu oznacza pojedynczy impuls.

Proponowane wartości domyślne:

```text
FORWARD/BACKWARD: ruch przez 0.25 sekundy
LEFT/RIGHT: obrót o 15 stopni albo impuls yaw przez 0.15 sekundy
JUMP: pojedyncze Jump()
```

Ważne: ruch ma być widoczny, ale nie zbyt długi. Student powinien móc budować ścieżkę przez powtarzanie komend.

Przykład:

```c
send_command("FORWARD");
send_command("FORWARD");
send_command("RIGHT");
send_command("FORWARD");
send_command("FORWARD");
```

---

## Projekt Unreal Engine

Bazą ma być:

```text
Games → Third Person → C++
```

Nie Blueprint-only, ponieważ potrzebujemy kodu C++ do wygodnego czytania FIFO.

Nazwa projektu:

```text
IpcCharacterWorld
```

Mapa startowa:

```text
ThirdPersonMap
```

Projekt powinien zawierać gotową postać z Third Person Template.

---

## Wymagania techniczne po stronie Unreal

Dodać komponent C++:

```text
UCommandReceiverComponent
```

albo aktora:

```text
ACommandReceiverActor
```

Rekomendacja: komponent przypięty do postaci gracza.

Odpowiedzialności komponentu:

1. Upewnia się, że FIFO istnieje.
2. Uruchamia wątek roboczy czytający FIFO.
3. Odbiera linie tekstu.
4. Parsuje komendy.
5. Przekazuje je bezpiecznie do Game Thread.
6. Wykonuje ruch na postaci.
7. Loguje odebrane i błędne komendy.

Nie wolno wywoływać metod Unreal Engine na obiektach gry bezpośrednio z wątku roboczego.  
Komendy odebrane w wątku roboczym trzeba przekazać do Game Thread, np. przez kolejkę i obsługę w `TickComponent`.

---

## Struktura klas Unreal

### `ECommandType`

Utworzyć enum:

```cpp
enum class ECommandType : uint8
{
    Forward,
    Backward,
    Left,
    Right,
    Jump,
    Stop,
    Reset,
    Ping,
    Unknown
};
```

### `FQueuedCommand`

```cpp
struct FQueuedCommand
{
    ECommandType Type;
    FString RawText;
    double ReceivedAtSeconds;
};
```

### `UCommandReceiverComponent`

Najważniejsze pola:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IPC")
FString PipePath = TEXT("/tmp/ue_character_commands");

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
float MoveImpulseSeconds = 0.25f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
float TurnDegrees = 15.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
float MoveStrength = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
bool bLogCommands = true;
```

Stan ruchu:

```cpp
float RemainingForwardTime = 0.0f;
float RemainingBackwardTime = 0.0f;
```

Kolejka:

```cpp
TQueue<FQueuedCommand, EQueueMode::Mpsc> PendingCommands;
```

Wątek:

- `FRunnable` albo `Async(EAsyncExecution::Thread, ...)`.
- Flaga atomowa `bShouldStop`.
- Wątek kończy się przy `EndPlay`.

---

## Tworzenie FIFO

Po stronie Unreal:

Jeśli `/tmp/ue_character_commands` nie istnieje, projekt ma je utworzyć.

Na Linuksie:

```cpp
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
```

Tworzenie:

```cpp
if (access(TCHAR_TO_UTF8(*PipePath), F_OK) != 0)
{
    mkfifo(TCHAR_TO_UTF8(*PipePath), 0666);
}
```

Czytanie powinno być nieblokujące albo obsłużone tak, żeby gra się nie zawiesiła.

Rekomendacja:

- FIFO otworzyć w wątku roboczym.
- Użyć `open(path, O_RDONLY | O_NONBLOCK)`.
- Czytać w pętli.
- Jeśli nie ma danych, `FPlatformProcess::Sleep(0.01f)`.
- Buforować znaki aż do `\n`.

Alternatywnie można otwierać FIFO cyklicznie, ale wersja z wątkiem i nieblokującym odczytem jest stabilniejsza.

---

## Obsługa zerwania połączenia

FIFO ma specyficzne zachowanie, gdy nie ma piszącego procesu. Implementacja ma być odporna na:

- brak procesu piszącego,
- zamknięcie deskryptora przez program studenta,
- puste odczyty,
- kilka komend wysłanych jednocześnie,
- komendy bez końcowego `\n`.

W przypadku `read()` zwracającego `0` lub `EAGAIN` gra ma dalej działać.

---

## Wykonywanie komend

W `TickComponent`:

1. Opróżnić `PendingCommands`.
2. Dla każdej komendy wykonać akcję.
3. Zaktualizować aktywne impulsy ruchu.

Przykład logiki:

```cpp
switch (Command.Type)
{
    case ECommandType::Forward:
        RemainingForwardTime += MoveImpulseSeconds;
        break;

    case ECommandType::Backward:
        RemainingBackwardTime += MoveImpulseSeconds;
        break;

    case ECommandType::Left:
        Character->AddControllerYawInput(-TurnDegrees);
        break;

    case ECommandType::Right:
        Character->AddControllerYawInput(TurnDegrees);
        break;

    case ECommandType::Jump:
        Character->Jump();
        break;

    case ECommandType::Stop:
        RemainingForwardTime = 0.0f;
        RemainingBackwardTime = 0.0f;
        break;

    case ECommandType::Reset:
        ResetCharacter();
        break;

    case ECommandType::Ping:
        UE_LOG(LogTemp, Display, TEXT("IPC PING received"));
        break;

    default:
        break;
}
```

Potem w każdym ticku:

```cpp
if (RemainingForwardTime > 0.0f)
{
    Character->AddMovementInput(Character->GetActorForwardVector(), MoveStrength);
    RemainingForwardTime -= DeltaTime;
}

if (RemainingBackwardTime > 0.0f)
{
    Character->AddMovementInput(-Character->GetActorForwardVector(), MoveStrength);
    RemainingBackwardTime -= DeltaTime;
}
```

---

## Reset postaci

Zapamiętać pozycję startową w `BeginPlay`:

```cpp
InitialTransform = Character->GetActorTransform();
```

Dla `RESET`:

```cpp
Character->SetActorTransform(InitialTransform);
RemainingForwardTime = 0.0f;
RemainingBackwardTime = 0.0f;
```

Dobrze też wyzerować velocity, jeśli CharacterMovementComponent jest dostępny:

```cpp
Character->GetCharacterMovement()->StopMovementImmediately();
```

---

## Integracja z Third Person Character

W klasie postaci dodać komponent:

```cpp
CommandReceiver = CreateDefaultSubobject<UCommandReceiverComponent>(TEXT("CommandReceiver"));
```

Jeżeli łatwiej, można też stworzyć osobnego aktora `ACommandReceiverActor`, który w `BeginPlay` znajdzie `UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)`.

Rekomendacja: komponent w postaci jest czystszy.

---

## Diagnostyka w grze

Dodać prosty tekst debugowy na ekranie:

- ścieżka FIFO,
- ostatnia odebrana komenda,
- liczba odebranych komend,
- liczba błędnych komend.

Można użyć:

```cpp
GEngine->AddOnScreenDebugMessage(...)
```

albo prostego Widget Blueprint.

Minimalna wersja może używać tylko `UE_LOG`, ale tekst na ekranie bardzo pomaga na zajęciach.

---

## Paczka dla studentów

Finalny katalog:

```text
lab-ipc-unreal/
├── Game/
│   ├── IpcCharacterWorld.sh
│   └── ...
├── examples/
│   ├── forward.c
│   ├── square.c
│   ├── jump.c
│   └── invalid_command.c
├── include/
│   └── ue_ipc.h
├── Makefile
├── README.md
└── TASKS.md
```

---

## Plik `include/ue_ipc.h`

Przygotować pomocniczy nagłówek dla przykładów, ale w zadaniach można wymagać, aby studenci samodzielnie napisali analogiczną funkcję.

```c
#ifndef UE_IPC_H
#define UE_IPC_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define UE_PIPE_PATH "/tmp/ue_character_commands"

static int send_command(const char *command)
{
    int fd = open(UE_PIPE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (write(fd, command, strlen(command)) < 0) {
        perror("write command");
        close(fd);
        return -1;
    }

    if (write(fd, "\n", 1) < 0) {
        perror("write newline");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

#endif
```

---

## Przykład `examples/forward.c`

```c
#include "../include/ue_ipc.h"
#include <unistd.h>

int main(void)
{
    send_command("RESET");
    sleep(1);

    send_command("FORWARD");
    usleep(300000);

    send_command("FORWARD");
    usleep(300000);

    send_command("FORWARD");
    usleep(300000);

    return 0;
}
```

---

## Przykład `examples/square.c`

```c
#include "../include/ue_ipc.h"
#include <unistd.h>

static void go_forward(int n)
{
    for (int i = 0; i < n; ++i) {
        send_command("FORWARD");
        usleep(300000);
    }
}

static void turn_right(int n)
{
    for (int i = 0; i < n; ++i) {
        send_command("RIGHT");
        usleep(200000);
    }
}

int main(void)
{
    send_command("RESET");
    sleep(1);

    for (int side = 0; side < 4; ++side) {
        go_forward(6);
        turn_right(6);
    }

    return 0;
}
```

---

## Przykład `examples/jump.c`

```c
#include "../include/ue_ipc.h"
#include <unistd.h>

int main(void)
{
    send_command("RESET");
    sleep(1);

    send_command("FORWARD");
    usleep(300000);

    send_command("JUMP");
    usleep(500000);

    send_command("FORWARD");
    usleep(300000);

    return 0;
}
```

---

## Makefile dla przykładów

```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2

EXAMPLES=forward square jump invalid_command

all: $(EXAMPLES)

forward: examples/forward.c include/ue_ipc.h
	$(CC) $(CFLAGS) examples/forward.c -o forward

square: examples/square.c include/ue_ipc.h
	$(CC) $(CFLAGS) examples/square.c -o square

jump: examples/jump.c include/ue_ipc.h
	$(CC) $(CFLAGS) examples/jump.c -o jump

invalid_command: examples/invalid_command.c include/ue_ipc.h
	$(CC) $(CFLAGS) examples/invalid_command.c -o invalid_command

clean:
	rm -f $(EXAMPLES)
```

---

## README dla studentów

README ma zawierać minimalną instrukcję:

```bash
# 1. Uruchom grę
./Game/IpcCharacterWorld.sh

# 2. W drugim terminalu skompiluj przykład
make

# 3. Uruchom przykład
./forward
./square
./jump

# 4. Test ręczny
echo FORWARD > /tmp/ue_character_commands
echo RIGHT > /tmp/ue_character_commands
```

W README wyjaśnić, że gra musi być uruchomiona przed wysyłaniem komend, bo FIFO może blokować zapis, jeśli nikt go nie czyta.

---

## TASKS.md dla studentów

Przygotować zadania:

### Zadanie 1

Napisz program `walk.c`, który wyśle 10 razy komendę `FORWARD`.

### Zadanie 2

Napisz program `turn.c`, który obróci postać w prawo o około 90 stopni, wysyłając kilka razy komendę `RIGHT`.

### Zadanie 3

Napisz program `rectangle.c`, który przeprowadzi postać po trasie przypominającej prostokąt.

### Zadanie 4

Napisz program `stairs.c`, który powtarza sekwencję:

```text
FORWARD
JUMP
FORWARD
```

### Zadanie 5

Napisz własną funkcję `send_command`, używając bezpośrednio:

```c
open()
write()
close()
```

Nie wolno używać `system("echo ...")`.

### Zadanie dodatkowe

Dodaj obsługę błędów:

- brak FIFO,
- gra nie jest uruchomiona,
- `open()` zwraca błąd,
- `write()` zwraca błąd.

---

## Automatyczne przygotowanie środowiska

Codex ma przygotować skrypty instalacyjne, ale musi uwzględnić, że Unreal Engine nie zawsze da się pobrać całkowicie automatycznie bez konta Epic/GitHub i zaakceptowania licencji.

### `scripts/install_linux_deps.sh`

Skrypt ma zainstalować zależności do kompilacji przykładów C oraz narzędzia pomocnicze:

```bash
#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y \
  build-essential \
  clang \
  lld \
  cmake \
  ninja-build \
  make \
  git \
  python3 \
  python3-pip \
  rsync \
  unzip \
  zip \
  dos2unix
```

### Unreal Engine

Codex ma obsłużyć dwa tryby:

#### Tryb A: Unreal Engine już zainstalowany

Użytkownik ustawia:

```bash
export UE_ROOT="$HOME/UnrealEngine"
```

albo:

```bash
export UE_ROOT="/opt/UnrealEngine"
```

Skrypty mają sprawdzić:

```bash
test -x "$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
```

Jeśli plik nie istnieje, wypisać czytelny komunikat.

#### Tryb B: pobranie Unreal Engine ze źródeł

Codex może przygotować instrukcję/skrypt, ale ma zaznaczyć, że użytkownik musi mieć konto Epic połączone z GitHub i dostęp do repozytorium EpicGames/UnrealEngine.

Przykładowy flow:

```bash
git clone --branch 5.6 https://github.com/EpicGames/UnrealEngine.git "$HOME/UnrealEngine"
cd "$HOME/UnrealEngine"
./Setup.sh
./GenerateProjectFiles.sh
make
```

Wersję `5.6` traktować jako domyślną, ale skrypt powinien pozwolić zmienić ją przez zmienną:

```bash
export UE_VERSION_BRANCH=5.6
```

Jeżeli repozytorium nie jest dostępne, skrypt ma wypisać instrukcję, że trzeba połączyć konto Epic z GitHub.

---

## Tworzenie projektu

Codex ma przygotować instrukcję i/lub skrypt:

```text
scripts/create_project.md
```

Ponieważ utworzenie Third Person Template może wymagać użycia Unreal Editor, dopuszczalny jest wariant półautomatyczny:

1. Otwórz Unreal Editor.
2. Utwórz nowy projekt:
   - Games
   - Third Person
   - C++
   - Desktop
   - Starter Content: off albo on, obojętnie
   - nazwa: `IpcCharacterWorld`
3. Zamknij edytor.
4. Uruchom skrypt Codexa, który doda klasy C++ i pliki pomocnicze.

Codex ma przygotować pliki:

```text
Source/IpcCharacterWorld/CommandReceiverComponent.h
Source/IpcCharacterWorld/CommandReceiverComponent.cpp
```

oraz zmodyfikować klasę Character, np.:

```text
Source/IpcCharacterWorld/IpcCharacterWorldCharacter.h
Source/IpcCharacterWorld/IpcCharacterWorldCharacter.cpp
```

---

## Build projektu Unreal

Przygotować skrypt:

```text
scripts/package_linux.sh
```

Założenia:

```bash
#!/usr/bin/env bash
set -euo pipefail

: "${UE_ROOT:?Set UE_ROOT to Unreal Engine root directory}"
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UPROJECT="$PROJECT_ROOT/IpcCharacterWorld.uproject"
OUT_DIR="$PROJECT_ROOT/Packaged/Linux"

"$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
  -project="$UPROJECT" \
  -noP4 \
  -platform=Linux \
  -clientconfig=Development \
  -serverconfig=Development \
  -cook \
  -allmaps \
  -build \
  -stage \
  -pak \
  -archive \
  -archivedirectory="$OUT_DIR"
```

---

## Skrypt uruchomieniowy gry

W paczce studenckiej utworzyć:

```text
Game/IpcCharacterWorld.sh
```

Ten skrypt ma:

1. Usunąć stare FIFO, jeśli istnieje i nie jest FIFO.
2. Utworzyć FIFO, jeśli nie istnieje.
3. Uruchomić grę.

Przykład:

```bash
#!/usr/bin/env bash
set -euo pipefail

PIPE="/tmp/ue_character_commands"

if [ -e "$PIPE" ] && [ ! -p "$PIPE" ]; then
  echo "Error: $PIPE exists but is not a FIFO"
  exit 1
fi

if [ ! -p "$PIPE" ]; then
  mkfifo "$PIPE"
  chmod 666 "$PIPE"
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$SCRIPT_DIR/IpcCharacterWorld/Binaries/Linux/IpcCharacterWorld" "$@"
```

Ścieżka do binarki może wymagać dopasowania do faktycznego outputu Unreal Package.

---

## Testy akceptacyjne

Projekt jest gotowy, jeśli przechodzi poniższe testy.

### Test 1: uruchomienie gry

```bash
./Game/IpcCharacterWorld.sh
```

Oczekiwane:

- gra się uruchamia,
- widoczna jest postać Third Person,
- nie ma crasha,
- FIFO istnieje:

```bash
ls -l /tmp/ue_character_commands
```

### Test 2: ręczne sterowanie z terminala

```bash
echo FORWARD > /tmp/ue_character_commands
echo RIGHT > /tmp/ue_character_commands
echo FORWARD > /tmp/ue_character_commands
```

Oczekiwane:

- postać porusza się,
- postać skręca,
- w logu widać odebrane komendy.

### Test 3: przykład C

```bash
make
./forward
```

Oczekiwane:

- postać idzie do przodu kilkoma impulsami.

### Test 4: błędna komenda

```bash
echo ABCD > /tmp/ue_character_commands
```

Oczekiwane:

- gra nie crashuje,
- log zawiera informację o nieznanej komendzie.

### Test 5: wiele komend

```bash
printf "RESET\nFORWARD\nFORWARD\nRIGHT\nFORWARD\n" > /tmp/ue_character_commands
```

Oczekiwane:

- gra przetwarza wszystkie linie.

---

## Wymagania jakościowe

- Gra nie może się zawieszać, gdy nikt nie pisze do FIFO.
- Gra nie może się zawieszać, gdy program C otworzy i zamknie FIFO.
- Komendy muszą działać z terminala i z programu C.
- Kod C++ ma być prosty i czytelny.
- Kod ma logować błędy.
- Paczka studencka ma nie wymagać instalacji Unreal Engine.
- Unreal Engine jest potrzebny tylko prowadzącemu / osobie przygotowującej paczkę.

---

## Dokumentacja do przygotowania

Codex ma utworzyć lub uzupełnić:

```text
README.md
TASKS.md
docs/TEACHER_GUIDE.md
docs/PROTOCOL.md
```

### `docs/PROTOCOL.md`

Opisać:

- ścieżkę FIFO,
- listę komend,
- format jednej komendy,
- przykłady,
- zachowanie przy błędnych komendach.

### `docs/TEACHER_GUIDE.md`

Opisać:

- jak zbudować projekt,
- jak spakować grę,
- jak rozdać paczkę studentom,
- jak testować,
- typowe problemy.

Typowe problemy:

1. `open: No such file or directory`  
   FIFO nie istnieje albo gra nie została uruchomiona.

2. `open()` blokuje program  
   Nikt nie czyta z FIFO. Uruchomić grę.

3. Brak ruchu w grze  
   Sprawdzić log Unreal, ścieżkę FIFO, uprawnienia i to, czy komenda kończy się `\n`.

4. Błąd uprawnień  
   Wykonać:

   ```bash
   chmod 666 /tmp/ue_character_commands
   ```

---

## Dodatkowe rozszerzenia na później

Nie implementować w pierwszej wersji, ale zostawić miejsce w projekcie:

- komunikacja przez UDP,
- shared memory,
- semafory,
- dwukierunkowa komunikacja: gra odsyła pozycję postaci,
- tryb zadania z labiryntem,
- detekcja kolizji z celem,
- punktacja,
- komenda `SAY <text>` do pokazania tekstu nad postacią.

---

## Priorytety implementacji

1. Działający Third Person Template.
2. FIFO `/tmp/ue_character_commands`.
3. Odczyt komend w Unreal bez blokowania gry.
4. Obsługa `FORWARD`, `RIGHT`, `LEFT`, `RESET`.
5. Przykłady C.
6. Pakowanie gry.
7. Dokumentacja.
8. Dopiero potem `JUMP`, debug UI i rozszerzenia.

---

## Uwagi dla Codexa

- Nie zakładaj, że Unreal Engine można pobrać bez interakcji użytkownika. Przygotuj skrypty, ale obsłuż brak dostępu do repozytorium EpicGames/UnrealEngine.
- Preferuj proste rozwiązania.
- Nie dodawaj zależności zewnętrznych po stronie Unreal, jeśli nie są konieczne.
- Nie wymagaj od studentów edytora Unreal.
- Całość ma być możliwa do demonstracji komendą `echo`.
- Najpierw przygotuj wersję minimalną, potem dopiero dodaj wygodniejsze narzędzia.
