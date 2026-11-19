# lab03-build-elf-memory

Lab 3 — Побудова програми, ELF та організація пам’яті.

## Мета роботи

Дослідити шлях перетворення C-коду на виконуваний файл, призначення linker та symbols, структуру ELF, секції `.text`, `.rodata`, `.data`, `.bss`, а також розміщення даних у пам’яті процесу.

## Середовище

Робота розрахована на Linux / GitHub Codespaces та компілятор GCC.

Перевірити інструменти:

```bash
gcc --version
readelf --version
objdump --version
```

## Структура проєкту

```text
lab03-build-elf-memory/
├── .gitignore
├── README.md
├── program.c
├── main.c
├── calculate.c
├── sections.c
├── process.c
├── memory.c
├── stack.c
├── heap.c
└── memory_map.c
```

## 1. Базова програма

Файл `program.c` містить глобальну ініціалізовану змінну `global_value`, глобальну неініціалізовану `global_empty` та функцію `add()`.

```bash
gcc -Wall -Wextra program.c -o program
./program
```

Очікуваний вивід:

```text
Result: 30
```

## 2. Етапи build pipeline

Команди потрібно виконувати з каталогу лабораторної:

```bash
gcc -E program.c -o program.i
gcc -S -O0 -masm=intel program.i -o program.s
gcc -c program.s -o program.o
gcc program.o -o program
```

Перевірка типів файлів:

```bash
file program.c program.i program.s program.o program
```

| Файл | Етап | Що містить | Чи виконується CPU безпосередньо? |
|---|---|---|---|
| `.c` | Початковий код | C source code | Ні |
| `.i` | Preprocessing | C-код після обробки директив та макросів | Ні |
| `.s` | Compilation | Assembly у текстовому вигляді | Ні |
| `.o` | Assembly | Машинний код, symbols та relocation information | Ні, зазвичай потребує linking |
| executable | Linking | ELF-файл, готовий до завантаження ОС | Так, після завантаження ОС |

### Відповіді

1. `program.i` більший за `program.c`, тому що препроцесор підставляє вміст підключених заголовків, зокрема `stdio.h`, та обробляє макроси й умовну компіляцію.
2. `#include <stdio.h>` замінюється вмістом відповідного заголовка та його залежностей.
3. `program.i` не є машинним кодом — це текстовий C-код після preprocessing.
4. CPU не може безпосередньо виконувати `program.i`; його ще потрібно скомпілювати та зібрати.
5. `program.s` — текстовий Assembly. У цій роботі `-masm=intel` задає синтаксис Intel, а цільова ISA залежить від архітектури середовища.
6. `program.o` є бінарним relocatable object file. `objdump -d -M intel program.o` показує дизасембльоване представлення машинних інструкцій.

## 3. Дизасемблювання та symbols

```bash
objdump -d -M intel program.o
nm program.o
```

У `nm` знайдіть `main`, `add`, `global_value`, `global_empty` та `printf`.

- `main` та `add` визначені в нашому object file.
- `global_value` і `global_empty` — глобальні об'єкти, визначені в цьому файлі.
- `printf` є зовнішнім символом: його реалізація надається C standard library, а не цим object file.

`symbol` — ім'я об'єкта або функції, яке використовується під час компіляції та linking. Linker зіставляє невизначені посилання з визначеннями в інших object files або libraries.

## 4. Linking та linker error

Спочатку зберіть `main.c` окремо:

```bash
gcc -c main.c -o main.o
gcc main.o -o app
```

Друга команда має завершитися помилкою `undefined reference to 'calculate'`, тому що `main.o` посилається на `calculate()`, але її реалізація не була передана linker-у.

Виправлення:

```bash
gcc -c calculate.c -o calculate.o
nm main.o
nm calculate.o
gcc main.o calculate.o -o app
./app
```

Очікуваний результат:

```text
30
```

Compilation пройшла, бо компілятору було достатньо оголошення `int calculate(int a, int b);`. Помилка виникла на етапі linking, коли linker не знайшов визначення функції.

## 5. Дослідження ELF

```bash
file program
readelf -h program
```

У `readelf -h` потрібно записати фактичні значення `Class`, `Data`, `Type`, `Machine` та `Entry point address`.

| ELF property | Результат у моєму середовищі |
|---|---|
| Class | Заповнити за `readelf -h program` |
| Data | Заповнити за `readelf -h program` |
| Type | Заповнити за `readelf -h program` |
| Machine | Заповнити за `readelf -h program` |
| Entry point | Заповнити за `readelf -h program` |

`ELF64` означає 64-бітний формат ELF. Поле `Machine` показує цільову архітектуру. Значення залежать від платформи, на якій виконувалася компіляція.

## 6. ELF sections

```bash
readelf -S program
```

| Section | Призначення та приклад у програмі |
|---|---|
| `.text` | Машинні інструкції функцій `main()` та `add()` |
| `.rodata` | Константні дані, наприклад рядок формату `printf` |
| `.data` | Глобальні та static змінні з ненульовими початковими значеннями, наприклад `global_value` |
| `.bss` | Глобальні та static змінні, неініціалізовані або ініціалізовані нулем, наприклад `global_empty` |

## 7. Експеримент із `.data` та `.bss`

```bash
gcc -Wall -Wextra sections.c -o sections
nm sections
readelf -S sections
objdump -s -j .data sections
objdump -s -j .rodata sections
size sections
```

Розміщення змінних:

| Об'єкт | Очікувана секція |
|---|---|
| `initialized` | `.data` |
| `uninitialized` | `.bss` |
| `static_initialized` | `.data` |
| `static_uninitialized` | `.bss` |
| `message` | `.rodata` |

### Перевірка великої `.bss`

Збережіть початковий `sections.c`, а потім замініть оголошення:

```c
int uninitialized;
```

на:

```c
int large_array[100000];
```

Повторіть:

```bash
gcc -Wall -Wextra sections.c -o sections
size sections
ls -lh sections
```

Масив із 100000 елементів `int` займає приблизно 400000 байтів (якщо `sizeof(int) == 4`). Він збільшує `.bss`, але не зобов'язаний так само збільшувати розмір executable-файлу: `.bss` зазвичай є `NOBITS` секцією та не зберігає всі нульові байти у файлі. Пам'ять для неї виділяється під час завантаження процесу.

У звіт потрібно вписати власні значення `text`, `data`, `bss` до та після зміни.

## 8. Dynamic libraries

```bash
ldd program
```

`ldd` показує shared libraries, від яких залежить executable. У типовому Linux-середовищі серед них буде C library, наприклад `libc.so.6`.

- Повна реалізація `printf()` зазвичай не копіюється в executable під час звичайного dynamic linking.
- `.so` означає shared object.
- Різні процеси можуть використовувати одну й ту саму shared library.
- Це зменшує дублювання коду у виконуваних файлах та дозволяє оновлювати бібліотеку окремо від програм (з урахуванням сумісності).

## 9. Executable та process

```bash
gcc -Wall -Wextra process.c -o process
./process
```

Програма виведе PID та чекатиме 60 секунд. В іншому терміналі виконайте:

```bash
ps -p PID
```

Замість `PID` підставте число з виводу програми.

Executable — це файл на диску. Process — запущений екземпляр програми, якому ОС надала віртуальний адресний простір, ресурси та PID.

## 10. Де знаходяться дані програми

```bash
gcc -Wall -Wextra -O0 memory.c -o memory
./memory
./memory
./memory
```

Прогноз розміщення:

| Об'єкт | Очікувана область |
|---|---|
| `main` | code (`.text`) |
| `global_initialized` | data (`.data`) |
| `global_uninitialized` | bss (`.bss`) |
| `static_initialized` | data (`.data`) |
| `static_uninitialized` | bss (`.bss`) |
| `local` | stack |
| пам'ять, отримана через `malloc` | heap |

Порівнювати потрібно групи адрес, а не конкретні числові значення. Global та static змінні зазвичай розташовані в областях даних executable, локальна змінна — у stack, а блок `malloc` — у heap або іншій області, якою керує allocator.

Адреси можуть змінюватися між запусками через ASLR (Address Space Layout Randomization). Це нормальна поведінка сучасної ОС.

## 11. Stack experiment

```bash
gcc -Wall -Wextra -O0 stack.c -o stack
./stack
```

Програма послідовно викликає `level1()`, `level2()` та `level3()`. Кожна функція має власну локальну змінну. На типовій x86-64 Linux системі stack росте в напрямку менших адрес, але не слід переносити це спостереження на всі архітектури та ABI без перевірки.

## 12. Heap experiment

```bash
gcc -Wall -Wextra heap.c -o heap
./heap
```

`malloc()` виділяє динамічну пам'ять. Адреси `a`, `b`, `c` можуть бути близькими, але не зобов'язані бути послідовними. Розміщення залежить від allocator та його внутрішньої організації.

## 13. Фінальна карта пам'яті

```bash
gcc -Wall -Wextra -O0 memory_map.c -o memory_map
./memory_map
```

Програма виводить адреси функції, глобальних і static змінних, рядкового літерала, heap allocation та локальної змінної.

Концептуальна карта (не точний масштаб і не повна карта Linux process address space):

```text
Higher addresses
+--------------------------+
|          Stack           |
|          local           |
+--------------------------+
|            ...           |
+--------------------------+
|           Heap           |
|      malloc allocation   |
+--------------------------+
|           .bss           |
| global_uninitialized     |
| static_uninitialized     |
+--------------------------+
|          .data           |
| global_initialized       |
| static_initialized       |
+--------------------------+
|         .rodata          |
| string literal           |
+--------------------------+
|          .text           |
| sample_function()        |
| main()                   |
+--------------------------+
Lower addresses
```

Фактичний порядок і адреси залежать від executable format, linker, loader, ASLR та runtime allocator. Карта показує концептуальне розділення областей, а не точні адреси з одного запуску.

## 14. Фінальна перевірка build pipeline

```bash
gcc -E memory_map.c -o memory_map.i
gcc -S -O0 -masm=intel memory_map.i -o memory_map.s
gcc -c memory_map.s -o memory_map.o
gcc memory_map.o -o memory_map

file memory_map.c
file memory_map.i
file memory_map.s
file memory_map.o
file memory_map
```

## 15. Контрольні питання

1. **Які основні етапи проходить C-програма до створення executable?**  
   Preprocessing, compilation, assembly та linking.

2. **Що робить preprocessor?**  
   Обробляє директиви `#include`, `#define`, умовну компіляцію та макроси.

3. **Що створює compiler?**  
   Перетворює C-код після preprocessing на Assembly-представлення або машинний код (залежно від режиму).

4. **Що робить assembler?**  
   Перетворює Assembly на object file з машинними інструкціями, symbols та relocation information.

5. **Що містить object file?**  
   Машинний код, дані, таблиці symbols, relocation information та метадані формату об'єктного файла.

6. **Навіщо потрібен linker?**  
   Об'єднує object files та libraries, розв'язує зовнішні посилання і створює executable або library.

7. **Що означає `undefined reference`?**  
   Під час linking не знайдено визначення символу, на який є посилання.

8. **Що таке symbol?**  
   Іменована функція, змінна або інший об'єкт, який фігурує в таблиці символів.

9. **Чим header file відрізняється від library?**  
   Header зазвичай містить оголошення та макроси для компілятора. Library містить реалізації функцій та об'єктний код для linker-а або dynamic loader-а.

10. **Що таке ELF?**  
    Executable and Linkable Format — формат виконуваних, об'єктних файлів та shared libraries у багатьох Unix-подібних системах.

11. **Що зазвичай містить `.text`?**  
    Машинний код програми.

12. **Що зазвичай містить `.rodata`?**  
    Дані, доступні лише для читання, наприклад рядкові літерали та константи.

13. **У чому різниця між `.data` та `.bss`?**  
    `.data` містить ініціалізовані дані, а `.bss` — неініціалізовані або нульові дані.

14. **Чому велика `.bss` не обов'язково означає такий самий приріст розміру executable?**  
    `.bss` зазвичай має тип `NOBITS`: її розмір описаний у ELF, але нульові байти не записуються в executable як звичайний вміст секції.

15. **Що показує `ldd`?**  
    Динамічні бібліотеки, потрібні executable, та шляхи до них у поточному середовищі.

16. **Чим executable відрізняється від process?**  
    Executable — файл, process — екземпляр програми, який виконується ОС.

17. **Де концептуально розташовуються local variables?**  
    Зазвичай у stack, хоча оптимізатор може зберігати деякі значення в регістрах.

18. **Для чого використовується heap?**  
    Для динамічного виділення пам'яті під час виконання програми.

19. **Яка різниця між stack та heap?**  
    Stack використовується для автоматичних даних викликів функцій і має автоматичне керування часом життя. Heap використовується для динамічних виділень, якими керує програма та allocator.

20. **Чому адреси можуть змінюватися між запусками програми?**  
    Через ASLR та особливості розміщення пам'яті процесу.

21. **Що таке temporal locality?**  
    Якщо до даних зверталися нещодавно, є висока ймовірність, що до них звернуться знову найближчим часом.

22. **Що таке spatial locality?**  
    Якщо звернулися до певної адреси, є висока ймовірність звернення до сусідніх адрес.

## Висновок

Під час лабораторної роботи досліджується повний шлях від C source code до ELF executable: preprocessing, compilation, assembly та linking. Команди `file`, `nm`, `objdump`, `readelf`, `ldd` і `size` дозволяють аналізувати формат файла, symbols, секції та залежності. Експерименти з адресами демонструють концептуальне розділення code, data, bss, stack і heap у віртуальному адресному просторі процесу.

**Примітка:** значення ELF header, `size`, `ldd` та адреси потрібно записати після запуску команд у власному GitHub Codespaces. Вони залежать від середовища, архітектури, версії компілятора та налаштувань ОС; вигадані результати в README не підставлялися.
