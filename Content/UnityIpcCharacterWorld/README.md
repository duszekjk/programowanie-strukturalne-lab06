# UnityIpcCharacterWorld

Kompletny projekt Unity do laboratorium z komunikacji międzyprocesowej.

Projekt jest przygotowany tak, żeby studenci nie musieli pisać nic w Unity. Po otwarciu katalogu `Content/UnityIpcCharacterWorld` w Unity scena zostanie utworzona automatycznie przez skrypt edytora:

- podłoga,
- kapsuła reprezentująca postać,
- kamera,
- światło,
- skrypt sterowania,
- odbiornik komend IPC.

## Wymagania

- Unity 2022.3 LTS albo nowsze,
- Linux/macOS dla FIFO `/tmp/unity_ipc_character_commands`,
- kompilator C, np. `gcc` albo `clang`.

UDP działa także na Windows, ale główny wariant zajęć zakłada FIFO na systemie Unix.

## Jak uruchomić

1. Otwórz Unity Hub.
2. Wybierz **Add project from disk**.
3. Wskaż katalog:

   ```text
   Content/UnityIpcCharacterWorld
   ```

4. Poczekaj na import skryptów.
5. Scena `Assets/Scenes/IpcLabScene.unity` zostanie utworzona automatycznie.
6. Otwórz scenę, jeżeli Unity nie zrobi tego samo.
7. Kliknij **Play**.

W trybie Play gra nasłuchuje równolegle:

- FIFO: `/tmp/unity_ipc_character_commands`,
- UDP: `127.0.0.1:7777`.

## Protokół komend

Każda komenda jest pojedynczą linią tekstu bez parametrów:

```text
FORWARD
BACKWARD
LEFT
RIGHT
JUMP
STOP
RESET
```

Komendy nie mają liczby ani czasu trwania. Jedna komenda oznacza jeden krótki krok/ruch. Jeżeli student chce wykonać ruch dwa razy, wysyła komendę dwa razy.

Przykład:

```text
FORWARD
FORWARD
RIGHT
FORWARD
JUMP
```

## Test z terminala przez FIFO

```bash
mkfifo /tmp/unity_ipc_character_commands 2>/dev/null || true
printf "FORWARD\n" > /tmp/unity_ipc_character_commands
printf "RIGHT\n" > /tmp/unity_ipc_character_commands
printf "FORWARD\n" > /tmp/unity_ipc_character_commands
```

## Test z terminala przez UDP

```bash
printf "FORWARD\n" | nc -u -w0 127.0.0.1 7777
printf "RIGHT\n" | nc -u -w0 127.0.0.1 7777
```

## Przykłady w C

W katalogu `examples/` są dwa warianty:

- `fifo_square.c` — wysyłanie komend przez FIFO,
- `udp_square.c` — wysyłanie komend przez UDP localhost.

Kompilacja:

```bash
cd examples
make
```

Uruchomienie:

```bash
./fifo_square
# albo
./udp_square
```

## Założenie dydaktyczne

Studenci piszą tylko program w C wysyłający tekstowe komendy. Projekt Unity jest gotowym odbiornikiem i wizualizacją. Dzięki temu zadanie dotyczy mechanizmu IPC, a nie tworzenia gry.
