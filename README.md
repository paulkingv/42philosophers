*This project was made as a part of the 42 curriculum by pking*
# Philosophers

## Description
This is a project focused on the famous **Dining Philosophers Problem**, a classic puzzle about sharing limited resources between several actors that all run at the same time.

**The setup:** a number of philosophers sit around a round table. In the middle is a big bowl of spaghetti. Between each pair of neighbors lies a single fork, so there are as many forks as philosophers. A philosopher can only eat when holding **both** the fork on their left and the fork on their right. Every philosopher repeats the same cycle: **eat → sleep → think**.

**The rules:**
- A philosopher who goes longer than `time_to_die` milliseconds without *starting* a meal dies, and the simulation ends.
- Philosophers cannot talk to each other and cannot know whether a neighbor is about to die.
- Every action is logged with a timestamp in milliseconds. A death must be announced within 10 ms of it happening.
- Optionally, the simulation can end successfully once every philosopher has eaten a given number of times.

**The challenge:** make it so that, for any number of philosophers, nobody dies when it's possible to avoid it. That means avoiding three classic concurrency problems:
- **Data races**: two threads reading/writing the same data at the same time.
- **Deadlock**: every philosopher holds one fork and waits forever for the other.
- **Starvation**: one philosopher keeps being skipped and never gets to eat.

**How this implementation works:**
- Each **philosopher is a thread** (`pthread_create`), and each **fork is a mutex** (`pthread_mutex_t`).
- To prevent deadlock, odd-numbered philosophers pick up their **left fork first** and even-numbered ones pick up their **right fork first**. Even-numbered philosophers also start half a meal late, so neighbors don't all grab for forks at the same instant.
- With an **odd** number of philosophers, each one "thinks" a little extra (`2 × time_to_eat − time_to_sleep`) so the table stays in a steady rhythm and nobody is starved.
- The **main thread is a monitor**. Every millisecond it checks whether any philosopher has exceeded `time_to_die`, and whether everyone has eaten enough times. When either happens it raises a shared **stop flag**, and all threads wind down.
- Shared data is protected by dedicated mutexes: one per fork, one per philosopher for their meal data (`last_meal`, `meals`), one for the stop flag, and one for printing. This keeps output lines from mixing and guarantees nothing is printed after a death.
- Sleeping is done in tiny slices (`ms_sleep`) so timing stays accurate and threads react immediately when the simulation stops.
- With a single philosopher there is only one fork, so that philosopher takes it, waits, and dies.
- At the end, all threads are joined, all mutexes are destroyed and all memory is freed.

## Instructions

### Requirements
- A C compiler (`cc`/`gcc`/`clang`) and `make`
- A POSIX system with pthreads (Linux or macOS)

### Compilation
From the root of the repository:

```bash
make          # builds the ./philo executable
make clean    # removes the object files
make fclean   # removes the object files and the executable
make re       # fclean + all
```

The code is compiled with `-Wall -Wextra -Werror -pthread`.

### Usage

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

| Argument | Meaning |
|---|---|
| `number_of_philosophers` | How many philosophers (and forks) are at the table |
| `time_to_die` (ms) | A philosopher dies if this much time passes since the start of their last meal (or the start of the simulation) |
| `time_to_eat` (ms) | How long a meal lasts. The philosopher holds both forks during this time |
| `time_to_sleep` (ms) | How long a philosopher sleeps after eating |
| `number_of_times_each_philosopher_must_eat` *(optional)* | If given, the simulation stops once every philosopher has eaten at least this many times. If omitted, it only stops when someone dies |

All arguments must be non-negative integers that fit in an `int` (an optional leading `+` is accepted). Invalid input (wrong number of arguments, letters, negative numbers, overflow) prints `Error` to standard error and exits with code `1`.

### Examples

```bash
./philo 1 800 200 200         # 1 philosopher: takes a fork and dies at ~800 ms
./philo 5 800 200 200         # nobody should die; runs until you press Ctrl+C
./philo 5 800 200 200 7       # stops once every philosopher has eaten 7 times
./philo 4 410 200 200         # nobody should die
./philo 4 310 200 100         # a philosopher dies at around 310 ms
```

### Output
Each line has the form `timestamp_in_ms philosopher_id message`, where the timestamp is the time elapsed since the start of the simulation:

```
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
200 1 is sleeping
400 1 is thinking
...
801 3 died
```

Possible messages: `has taken a fork`, `is eating`, `is sleeping`, `is thinking`, `died`.

### Debugging (optional)
The Makefile can build with a sanitizer. Never combine both, and run `make fclean` when switching:

```bash
make re SAN=thread     # ThreadSanitizer: detects data races
make re SAN=address    # AddressSanitizer: detects memory errors
```

On Linux you can also use `valgrind --leak-check=full ./philo ...` and `valgrind --tool=helgrind ./philo ...`.

## Resources

## LLM Usage:
LLMs were used for the following:

- Scaffolding the project
- Teaching me about the external functions that I had never used before
- Advising on logic for how threads work
- This README.md
