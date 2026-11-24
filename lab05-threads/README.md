# lab05-threads

Lab 5 — Потоки, Race Conditions та Mutex.

## Мета роботи

Дослідити модель з кількома threads в одному process: спільну пам'ять, власні stack, nondeterministic execution, race condition (lost update), виправлення через mutex, deadlock та проєктування паралельної програми з мінімумом shared mutable state.

## Середовище

Linux / GitHub Codespaces, GCC, POSIX Threads. Усі програми компілюються так:

```bash
gcc -Wall -Wextra -pthread program.c -o program
```

Опція `-pthread` повідомляє GCC, що програма використовує POSIX Threads.

## Структура проєкту

```text
lab05-threads/
├── .gitignore
├── README.md
├── first_thread.c
├── multiple_threads.c
├── shared_data.c
├── thread_stacks.c
├── race.c
├── mutex_counter.c
├── race_four.c
├── mutex_four.c
├── shared_heap.c
├── thread_result.c
├── deadlock.c
├── deadlock_fixed.c
├── parallel_sum.c
└── sequential_sum.c
```

`deadlock_fixed.c` — виправлена версія `deadlock.c` (завдання 16), `sequential_sum.c` — для додаткового завдання.

---

## 1. Перший thread (`first_thread.c`)

Вивід:

```text
Main: creating thread
Hello from worker thread!
Main: worker finished
```

### Відповіді

1. **Скільки processes створила програма?** Один. `pthread_create()` не створює process.
2. **Скільки threads після успішного `pthread_create()`?** Два: main thread і worker thread.
3. **Яка функція виконується новим thread?** `worker()`.
4. **Для чого `pthread_join()`?** Main чекає завершення worker і може отримати його результат. Без join main міг би завершитись раніше, а разом із ним завершився б увесь process.
5. **Чи створює `pthread_create()` новий address space?** Ні. Новий thread працює в тому самому address space; створюється лише новий stack та execution context.

---

## 2. Декілька threads (`multiple_threads.c`)

Результати п'яти запусків (порядок появи повідомлень):

| Run | Порядок |
|---|---|
| 1 | 4, 1, 2, 3 |
| 2 | 1, 4, 2, 3 |
| 3 | 1, 2, 4, 3 |
| 4 | 1, 2, 3, 4 |
| 5 | 1, 2, 3, 4 |

**Чи гарантує цикл `pthread_create()` порядок 1 → 2 → 3 → 4?** Ні. Створення thread лише повідомляє ОС, що є новий thread, готовий до виконання. Коли саме він отримає CPU, вирішує scheduler: це залежить від навантаження системи, кількості ядер і моменту, коли thread потрапив у чергу. Тому thread, створений пізніше, може надрукувати повідомлення раніше (як у run 1, де першим був Thread 4). Це nondeterministic execution.

---

## 3. Shared global variable (`shared_data.c`)

Вивід одного з запусків:

```text
Thread 2: value=200, address=0x55bdb3db1014
Thread 1: value=100, address=0x55bdb3db1014
Main: value=100, address=0x55bdb3db1014
```

| Хто | Адреса `value` |
|---|---|
| Thread 1 | `0x55bdb3db1014` |
| Thread 2 | `0x55bdb3db1014` |
| Main | `0x55bdb3db1014` |

(Між запусками адреса змінюється через ASLR, але в межах одного запуску вона однакова для всіх threads.)

### Відповіді

- **Чи однакова адреса `value` для threads?** Так.
- **Чому?** Глобальна змінна живе в секції `.data` / `.bss` process, а вона одна на весь process.
- **Чи мають threads одного process спільний address space?** Так: code, data, heap, відкриті файли тощо.
- **Чому фінальне значення залежить від порядку виконання?** Обидва threads пишуть у ту саму змінну; виграє той, хто записав останнім. У виводі вище останнім записав Thread 1, тому `Main: value=100`.

Порівняння з Lab 4:

```text
Processes (fork):               Threads:

Process A     Process B         Thread A ──┐
  value         value                      ├──→ shared value
                                Thread B ──┘
separate address spaces
```

---

## 4. Власний stack (`thread_stacks.c`)

Вивід:

```text
Thread 2: local=20, address=0x7f6dd5ffdeb0
Thread 1: local=10, address=0x7f6dd67feeb0
```

| Хто | Адреса `local` |
|---|---|
| Thread 1 | `0x7f6dd67feeb0` |
| Thread 2 | `0x7f6dd5ffdeb0` |

Адреси відрізняються приблизно на 8 МБ (`0x7ff000`), що відповідає типовому розміру stack thread на Linux.

**Чому глобальна змінна shared, а локальні мають різні адреси?** Глобальна змінна належить process і лежить в одному місці. Локальна змінна лежить у stack виклику функції, а кожен thread має власний stack, тому кожен thread отримує власну копію `local` за власною адресою.

```text
Process
│
├── Code       ← shared
├── Data       ← shared
├── Heap       ← shared
│
├── Thread 1
│     └── Stack 1
│
└── Thread 2
      └── Stack 2
```

---

## 5. Race condition (`race.c`)

Очікуване значення: `2000000`.

| Run | Expected | Actual |
|---|---|---|
| 1 | 2000000 | 1000000 |
| 2 | 2000000 | 2000000 |
| 3 | 2000000 | 2000000 |
| 4 | 2000000 | 1331172 |
| 5 | 2000000 | 1013128 |

Додатково: у серії з 300 запусків `race` дала неправильний результат у 90 випадках (30 %). Тобто в частині запусків результат правильний, але це **не доводить**, що програма коректна. Мільйон ітерацій виконується лише за кілька мілісекунд, тому іноді перший thread встигає завершитись ще до того, як другий почне працювати, і перетину в часі немає. Якщо збільшити `ITERATIONS` до 20 000 000, помилка з'являється в кожному запуску (приблизно 20.2–22.8 млн замість 40 млн).

### Чому `counter++` створює проблему

Концептуально це три кроки:

```text
READ counter → ADD 1 → WRITE counter
```

Приклад lost update при `counter = 100`:

```text
Thread A                 Thread B

READ 100
                         READ 100

ADD → 101
                         ADD → 101

WRITE 101
                         WRITE 101
```

Очікували `102`, отримали `101`. Одне збільшення втрачено. Це **lost update**.

### Machine instructions (`gcc -O0 -S -masm=intel race.c`)

Фрагмент `counter++` у функції `increment`:

```asm
.L3:
    mov     rax, QWORD PTR counter[rip]    ; READ  counter -> rax
    add     rax, 1                         ; MODIFY (в регістрі)
    mov     QWORD PTR counter[rip], rax    ; WRITE rax -> counter
    add     QWORD PTR -8[rbp], 1           ; i++
.L2:
    cmp     QWORD PTR -8[rbp], 999999
    jle     .L3
```

**Чи виглядає `counter++` як одна неподільна операція?** Ні. Один рядок C перетворився на три окремі інструкції: `mov` (читання), `add`, `mov` (запис). Scheduler може перервати thread між будь-якими двома з них, і тоді другий thread прочитає старе значення. Це зв'язок із Module 2:

```text
C statement  →  machine instructions  →  CPU execution
 counter++       mov / add / mov          можна перервати між ними
```

### Аналіз програми

```text
Shared state:       глобальна змінна counter
Concurrent writers: thread1 і thread2 (функція increment)
Critical operation: counter++ (READ + MODIFY + WRITE)
```

---

## 6. Виправлення за допомогою mutex (`mutex_counter.c`)

| Run | race.c | mutex_counter.c |
|---|---|---|
| 1 | 1000000 | 2000000 |
| 2 | 2000000 | 2000000 |
| 3 | 2000000 | 2000000 |
| 4 | 1331172 | 2000000 |
| 5 | 1013128 | 2000000 |

**Чому mutex змінив поведінку?** `pthread_mutex_lock()` дозволяє зайти в critical section лише одному thread. Якщо mutex уже належить іншому thread, той, що викликав lock, блокується й чекає. Завдяки цьому послідовність READ → ADD → WRITE одного thread не перемішується з такою ж послідовністю іншого (**mutual exclusion**), і lost update неможливий.

```text
Thread A: LOCK → counter++ → UNLOCK
Thread B: LOCK → (чекає, поки A володіє mutex) → counter++ → UNLOCK
```

**Навіщо `pthread_mutex_unlock()`?** Він звільняє mutex. Без нього інші threads чекали б вічно.

### Де ставити lock

Варіант 1: lock/unlock всередині циклу на кожній ітерації. Критична секція мінімальна, але lock береться 2 млн разів.

Варіант 2: один lock навколо всього циклу. Це теж коректно, але Thread A тримає mutex на весь мільйон інкрементів, а Thread B весь цей час чекає. Паралелізму фактично немає: виконання стає послідовним.

```text
Thread A  ├──────── holds mutex ────────┤
Thread B  └──────── WAIT ───────────────┘ потім працює
```

**Висновок:** critical section має бути достатньою для correctness, але без потреби не повинна бути надмірно великою.

---

## 7. Вартість synchronization

Час виконання (`time`, секунди, приблизно, 3 запуски):

| Program | Run 1 | Run 2 | Run 3 |
|---|---|---|---|
| race | 0.007 | 0.007 | 0.008 |
| mutex_counter | 0.123 | 0.206 | 0.179 |

Це не точний benchmark, але різниця на порядок видна.

**Чому synchronized program повільніша?** Кожен lock/unlock — це додаткові операції (atomic-інструкції, синхронізація кешів між ядрами), а при contention thread може засинати й прокидатись через ядро ОС. Mutex купує correctness ціною overhead та lock contention.

```text
Synchronization → correctness + overhead / contention
```

---

## 8. Більше threads (`race_four.c`, `mutex_four.c`)

4 threads × 500 000 інкрементів, очікувано 2 000 000.

| Run | race_four (без mutex) | mutex_four |
|---|---|---|
| 1 | 1009664 | 2000000 |
| 2 | 1014202 | 2000000 |
| 3 | 1112166 | 2000000 |
| 4 | 1325840 | 2000000 |
| 5 | 626308 | 2000000 |

Порівняння:

| Варіант | Коректність | Типовий час |
|---|---|---|
| 2 threads без mutex | іноді правильно (випадково), у 30 % запусків ні | ~0.007 с |
| 4 threads без mutex | неправильно в усіх запусках | ~0.007 с |
| 2 threads з mutex | завжди правильно | ~0.12–0.21 с |
| 4 threads з mutex | завжди правильно | ~0.16–0.18 с |

**Чи означає більше threads автоматично швидшу програму?** Ні. Тут усі threads змагаються за один counter. З mutex вони по черзі чекають один одного, тож додаткові threads лише збільшують contention. Без mutex програма швидка, але дає неправильний результат. Також корисна робота має бути достатньо великою, щоб окупити витрати на створення threads, а кількість ядер обмежує реальний паралелізм (на машині, де я запускав, лише 2 ядра).

---

## 9. Shared heap (`shared_heap.c`)

Вивід:

```text
Worker: value=110 address=0x558b361352a0
Main: value=110 address=0x558b361352a0
```

- **Де знаходиться `data`?** Структура виділена через `malloc`, тобто в **heap**. Сам вказівник `data` (змінна) лежить у stack функції `main`, але вказує на heap.
- **Чи бачить worker ту саму пам'ять?** Так: адреса однакова, а main після `join` бачить `110`.
- **Чому?** Heap належить process і спільний для всіх threads. Thread отримав лише вказівник на ту саму область.
- **Що б змінилося, якби кілька threads одночасно робили `data->value += 10`?** Це теж READ + MODIFY + WRITE, тому виник би той самий race condition і lost update. Потрібен mutex.

---

## 10. Повернення результату (`thread_result.c`)

Вивід:

```text
Result = 144
```

**Чому не можна написати `int result = value * value; return &result;`?** `result` — локальна змінна, вона лежить у stack thread. Коли функція завершується, її stack frame перестає бути дійсним, а вказівник `&result` стає dangling pointer (undefined behavior). Тому результат виділяється в heap через `malloc`: така пам'ять живе, доки її не звільнять, і main після `pthread_join` може прочитати значення та викликати `free`. Це перевіряє розуміння thread stack, heap, pointer і object lifetime.

---

## 11. Deadlock

### Аналіз до запуску

```text
Thread A                  Thread B

lock mutex1               lock mutex2
      │                         │
      ▼                         ▼
 lock mutex2  ← зайнятий   lock mutex1  ← зайнятий
 (чекає B)                 (чекає A)

             DEADLOCK
```

Thread A тримає `mutex1` і чекає `mutex2`. Thread B тримає `mutex2` і чекає `mutex1`. Ніхто не може продовжити й відпустити свій mutex, тому обидва чекають вічно.

### Спостереження (`deadlock.c`)

Програма зависла після такого виводу (зупинена через 4 секунди `timeout`, exit code 124):

```text
Thread 1: locking mutex1
Thread 2: locking mutex2
Thread 1: locking mutex2
Thread 2: locking mutex1
```

Обидва threads спробували взяти другий mutex, але повідомлення `critical section` так і не з'явилося. Примітка: якщо stdout перенаправлений у pipe або файл, вивід буферизується і при примусовій зупинці може не з'явитися взагалі, тому запускав через `stdbuf -oL ./deadlock`.

### Circular wait

```text
Thread 1 → owns mutex1 → waits for mutex2
              ▲                  │
              │                  ▼
         waits for mutex1 ← Thread 2 owns mutex2
```

```text
Thread 1 → Mutex 2 → Thread 2 → Mutex 1 → Thread 1
```

### Виправлення (`deadlock_fixed.c`)

Обидва threads беруть locks в одному порядку: спочатку `mutex1`, потім `mutex2`.

```text
Thread 1: mutex1 → mutex2
Thread 2: mutex1 → mutex2
```

Результат (програма завершується, exit code 0):

```text
Thread 1: locking mutex1
Thread 2: locking mutex1
Thread 1: locking mutex2
Thread 1: critical section
Thread 2: locking mutex2
Thread 2: critical section
```

**Яку умову deadlock усунули?** **Circular wait.** Якщо всі threads беруть locks у фіксованому порядку, цикл очікування утворитись не може: той, хто не отримав `mutex1`, ще нічого не тримає.

---

## 12. Race Condition ≠ Deadlock

| Характеристика | Race Condition | Deadlock |
|---|---|---|
| Threads продовжують execution? | Так | Ні, заблоковані threads стоять |
| Результат може бути неправильним? | Так | Результату немає взагалі |
| Програма може повністю перестати прогресувати? | Ні | Так |
| Пов'язано з timing/order? | Так | Так (потрібен певний порядок захоплення) |
| Mutex може допомогти? | Так | Ні, mutex якраз і є його причиною |
| Неправильне використання mutex може бути причиною? | Так (lock не навколо всього критичного коду) | Так (різний порядок locks) |

```text
No synchronization        → Race Condition
Incorrect synchronization → Deadlock
```

Synchronization потрібна, але її теж треба проєктувати правильно.

---

## 13. Фінальне завдання — Parallel Sum (`parallel_sum.c`)

Сума `1 + 2 + ... + 1 000 000` на 4 threads. Кожен thread отримує власний `Task` (`start`, `end`, `result`) і рахує локальну суму без mutex.

Вивід:

```text
Thread 1: 1 ... 250000 = 31250125000
Thread 2: 250001 ... 500000 = 93750125000
Thread 3: 500001 ... 750000 = 156250125000
Thread 4: 750001 ... 1000000 = 218750125000

Parallel result: 500000500000
Expected result: 500000500000

Correct: YES
```

Формула перевірки: `N × (N + 1) / 2 = 1000000 × 1000001 / 2 = 500000500000`.

### Аналіз

1. **Які data shared?** Масив `tasks` (доступний main і worker threads), а також code і heap/data process. Кожен worker пише лише у свій елемент масиву.
2. **Які data thread-local?** Змінна `sum` усередині `calculate_sum()` та лічильник циклу `i` (у stack кожного thread).
3. **Чому mutex у циклі не потрібен?** Кожен thread змінює лише власний local accumulator і власний `Task`, тож два threads ніколи не пишуть в одну пам'ять.
4. **Коли main читає результати?** Після `pthread_join()` відповідного thread. Join гарантує, що thread завершився і його запис у `result` уже виконаний.
5. **Чому це краще за глобальний counter з lock на кожній ітерації?** Спільного змінюваного стану майже немає, тому немає ні race condition, ні contention, ні 1 млн lock/unlock. Синхронізація лише одна: `join` в кінці.

**Архітектурний урок:** найкращий спосіб виправити race condition іноді полягає не в додаванні більшої кількості locks, а в зменшенні shared mutable state.

### Додаткове завдання: sequential vs 4 threads

| Program | Run 1 | Run 2 | Run 3 |
|---|---|---|---|
| sequential_sum | 0.004 | 0.004 | 0.005 |
| parallel_sum | 0.004 | 0.003 | 0.004 |

Для N = 1 000 000 різниці практично немає: робота займає кілька мілісекунд, а створення threads теж коштує часу. **Коли parallelization має сенс?** Коли задача достатньо велика (обчислення значно дорожче за створення threads), її можна розбити на незалежні частини з малою кількістю shared state, а в системі є кілька вільних ядер CPU.

---

## Контрольні питання

1. **Що таке thread?** Одиниця виконання всередині process зі своїм stack, регістрами та instruction pointer.
2. **Чим thread відрізняється від process?** Process має власний address space, threads одного process діляться ним. Створити та перемкнути thread дешевше.
3. **Що threads одного process поділяють?** Code, глобальні дані (`.data`, `.bss`), heap, відкриті файли.
4. **Що кожен thread має власне?** Stack, значення регістрів, program counter, локальні змінні.
5. **Чи має кожен thread власний stack?** Так.
6. **Чи мають threads спільний heap?** Так.
7. **Для чого `pthread_create()`?** Створює новий thread, який виконує вказану функцію з переданим аргументом.
8. **Для чого `pthread_join()`?** Чекає завершення thread і забирає його return value.
9. **Чи гарантується порядок виконання порядком `pthread_create()`?** Ні, порядок визначає scheduler.
10. **Чим concurrency відрізняється від parallelism?** Concurrency — це кілька задач у процесі виконання з перекриттям у часі (можливо на одному ядрі, по черзі). Parallelism — задачі справді виконуються одночасно на різних ядрах.
11. **Що таке shared state?** Дані, до яких мають доступ кілька threads одночасно (глобальні змінні, heap).
12. **Що таке race condition?** Ситуація, коли результат залежить від порядку або timing доступу кількох threads до shared state без синхронізації.
13. **Що таке lost update?** Коли одне з оновлень перезаписується іншим, бо обидва threads прочитали старе значення.
14. **Чому `counter++` не atomic?** Це три інструкції (load, add, store), між якими thread можна перервати.
15. **Що таке critical section?** Ділянка коду, що працює із shared state і не повинна виконуватись кількома threads одночасно.
16. **Що таке mutual exclusion?** Гарантія, що в critical section одночасно є щонайбільше один thread.
17. **Що таке mutex?** Об'єкт синхронізації (lock), що забезпечує mutual exclusion; у POSIX це `pthread_mutex_t`.
18. **Що робить `pthread_mutex_lock()`?** Захоплює mutex; якщо він вільний, thread стає його власником і продовжує виконання.
19. **Що, якщо mutex належить іншому thread?** Викликаючий thread блокується, доки mutex не звільниться.
20. **Навіщо `pthread_mutex_unlock()`?** Щоб звільнити mutex і дати іншим threads увійти в critical section; без цього вони чекатимуть вічно.
21. **Чому надто велика critical section погіршує concurrency?** Інші threads довго чекають, і програма стає фактично послідовною.
22. **Що таке lock contention?** Ситуація, коли кілька threads часто конкурують за один і той самий lock і змушені чекати.
23. **Чому більше threads ≠ більша performance?** Через contention, overhead створення і перемикання threads, обмежену кількість ядер та частину коду, що не розпаралелюється.
24. **Що таке deadlock?** Стан, коли threads назавжди заблоковані, бо кожен чекає ресурс, який тримає інший.
25. **Чим deadlock відрізняється від race condition?** При race condition програма працює, але може дати неправильний результат; при deadlock програма зупиняється й не прогресує.
26. **Що таке circular wait?** Замкнений ланцюг очікування: T1 чекає ресурс T2, T2 чекає ресурс T1 (або довший цикл).
27. **Як однаковий порядок locks допомагає уникнути deadlock?** Усуває circular wait: цикл очікування неможливий, якщо всі беруть locks в одному порядку.
28. **Що таке starvation?** Коли thread дуже довго (або нескінченно) не отримує потрібний ресурс чи CPU, бо його постійно випереджають інші.
29. **Чому local accumulation краще за shared counter з mutex на кожній ітерації?** Немає contention і майже немає синхронізації: threads працюють незалежно, а результати об'єднуються один раз після `join`.
30. **Чому 100 успішних запусків не доводять відсутність race condition?** Race залежить від timing; на практиці він може проявлятись рідко. У цій роботі `race` давала правильний результат у 70 % запусків, хоч програма некоректна. Відсутність помилки в тестах не доводить коректність.

---

## Висновок

У роботі показано, що threads одного process ділять code, глобальні дані та heap, але мають власні stack. Нескоординований доступ кількох threads до однієї змінної призводить до race condition: `counter++` розкладається на кілька машинних інструкцій, і оновлення губляться (lost update). Mutex забезпечує mutual exclusion та правильний результат, але додає overhead (у цих експериментах приблизно в 20–30 разів повільніше) і не прискорюється від додавання threads. Неправильний порядок захоплення кількох mutex призводить до deadlock, який усувається єдиним порядком locks. Найкращий дизайн, як у `parallel_sum`, — зменшувати shared mutable state: кожен thread працює зі своїми даними, а результати об'єднуються після `pthread_join()`.
