# Philosophers: Your Study Guide for Corrections

Hello, future-you reading this tonight. First, a deep breath.

You are *not* starting from zero. I read every file, built the project, and ran it
through the usual evaluation scenarios, thread-sanitizer, address-sanitizer and a leak
check. **It works.** (Details in [section 11](#11-what-i-actually-tested).) What's
missing is not the code, it's the story in your head about *why* the code looks the way
it does. That is very fixable in one evening, because the whole project is only ~700
lines and it has one simple idea at its heart.

This guide teaches that idea slowly, with pictures and metaphors, then gives you the
questions a corrector is likely to ask, with answers you can say out loud in your own
words.

---

## Table of contents

0. [A gentle plan for tonight](#0-a-gentle-plan-for-tonight)
1. [The 60-second version (say this out loud)](#1-the-60-second-version)
2. [The big idea, with metaphors](#2-the-big-idea-with-metaphors)
3. [The program from above](#3-the-program-from-above)
4. [File-by-file tour](#4-file-by-file-tour)
5. [Follow one philosopher through a meal](#5-follow-one-philosopher-through-a-meal)
6. [The four kinds of lock, and what each protects](#6-the-four-kinds-of-lock)
7. [Three traps and how the code dodges them](#7-three-traps-and-how-the-code-dodges-them)
8. [The borrowed functions cheat sheet](#8-the-borrowed-functions-cheat-sheet)
9. [Questions a corrector might ask (with answers)](#9-questions-a-corrector-might-ask)
10. [Honest weak spots (so nothing surprises you)](#10-honest-weak-spots)
11. [What I actually tested](#11-what-i-actually-tested)
12. [Commands to try yourself tonight](#12-commands-to-try-yourself-tonight)
13. [Self-quiz](#13-self-quiz)

---

## 0. A gentle plan for tonight

You don't need to memorize anything. You need to be able to *explain it in your own
words*, and to *find things in your code quickly*. Here's a kind, realistic schedule
(about 2 hours, with a break):

| Time | What | Why |
|------|------|-----|
| 15 min | Read sections 1-3 | Get the picture in your head first |
| 25 min | Open the code, read it in the order of section 4 with this guide beside it | Now the code will feel familiar instead of foreign |
| 15 min | Section 5 + 6 + 7 (the heart of it) | This is what correctors care about most |
| 10 min | Break. Water. Walk. | Your brain files things away while resting |
| 20 min | Section 12: run the commands yourself, watch the output | Seeing it run makes it real |
| 25 min | Sections 9 and 13: answer the questions *out loud* | Saying it aloud is the best practice for a spoken evaluation |
| 10 min | Section 10: skim the weak spots, decide what you'd say | No surprises tomorrow |

If you run short on time, the **must-read** sections are **1, 2, 6, 7 and 9**.

A note on honesty: it's completely okay that an AI helped you scaffold this (your README
says so too). The goal tonight is just to make the ideas yours. Everything below is
written so you can do exactly that.

---

## 1. The 60-second version

> "My program simulates the Dining Philosophers problem. Each philosopher is a **thread**
> and each fork is a **mutex**. A philosopher needs two forks to eat, then sleeps, then
> thinks, and repeats. To avoid **deadlock**, odd philosophers pick up their left fork
> first and even philosophers their right fork first, and even philosophers start a
> little late so they don't all grab at once. The **main thread acts as a monitor**: every
> millisecond it checks whether anyone has gone longer than `time_to_die` without eating,
> and whether everyone has eaten enough times. When either happens it raises a **stop
> flag**, which every thread checks, so they all wind down, and then I **join** the
> threads, destroy the mutexes and free the memory. Shared data, like the stop flag and
> each philosopher's last meal time, is protected by mutexes so there are no **data
> races**."

If you can say that paragraph, you already understand 80% of the project.

---

## 2. The big idea, with metaphors

Picture a **round dinner table** with N philosophers and N forks. One fork sits between
each pair of neighbors. In the middle: a big bowl of spaghetti. To eat, a philosopher must
hold **both** the fork on their left and the fork on their right.

Everyone repeats: **eat → sleep → think**. And if someone goes too long without eating,
they die and the dinner ends.

Here's how the programming words map onto the dinner:

| Programming word | Dinner metaphor | In your code |
|---|---|---|
| **Thread** | A person who acts on their own, at the same time as everyone else | One thread per philosopher (`pthread_create` in `init.c`) |
| **Mutex** | A single key (or a fork!). Only one person can hold it. Everyone else queues politely until it's put down | `pthread_mutex_t`; `forks[i]`, `print_lock`, `stop_lock`, `meal_lock` |
| **Lock / unlock** | Picking up / putting down the key | `pthread_mutex_lock` / `pthread_mutex_unlock` |
| **Data race** | Two people scribbling on the same sticky note at the same moment: the note ends up garbled | Reading `last_meal` while a philosopher is writing it, if there were no lock |
| **Deadlock** | Everybody politely holding their left fork, each waiting for the right fork, forever. Like four cars at a four-way stop where each waits for the car on its right | Avoided by alternating fork order |
| **Starvation** | One unlucky philosopher keeps getting skipped and never eats | Avoided by staggering + the "extra think" for odd tables |
| **Monitor** | A waiter walking around with a stopwatch, checking when each guest last ate | `monitor()` in `monitor.c`, running on the main thread |
| **Stop flag** | The "closing time" bell. When it rings, everyone puts their forks down and leaves | `data->stop`, guarded by `stop_lock` |
| **`pthread_join`** | Waiting by the door until every guest has left before turning off the lights | `cleanup()` |
| **Timestamp in ms** | The stopwatch on the wall | `now_ms()` using `gettimeofday` |

### Why is this even hard?

Because the threads all run **at once**, and the order in which the computer runs them
is not something you control. Imagine five people reaching for forks with their eyes
closed. If you don't set rules, you can get chaos (data races), standstill (deadlock),
or unfairness (starvation). The whole project is about setting *just enough* rules.

---

## 3. The program from above

```
main()                                              [main.c]
 │
 ├─ init_data()           zero the struct, set "no meal limit" (-1)      [init.c]
 ├─ parse()               are the arguments the right count & digits?    [parse.c]
 ├─ convert_to_ascii()    turn the strings into ints, store in t_data    [parse.c]
 │
 ├─ init_simulation()                                                    [init.c]
 │    ├─ init_mutexes()   malloc forks[] + philo[], init every mutex
 │    ├─ now_ms()         remember the start time
 │    ├─ init_philos()    give each philo an id, two forks, last_meal
 │    └─ pthread_create() × N     ◄── N philosophers are now running at once
 │
 ├─ monitor()             MAIN thread becomes the waiter                 [monitor.c]
 │    loop every 1 ms:  anyone dead?  everyone fed?  → raise stop flag
 │
 └─ cleanup()             join all threads, destroy mutexes, free        [cleanup.c]
```

And meanwhile, on each of the N philosopher threads:

```
philo_routine()                                                  [philo_routine.c]
   (one philosopher alone?  →  grab the fork, wait to die)
   (even id?                →  wait time_to_eat / 2 first)
   while not stopped:
        eat()    →  take_forks → stamp last_meal → "is eating" → sleep → put forks down
        "is sleeping" → ms_sleep(time_to_sleep)
        think()  →  "is thinking" (+ extra waiting if the table is odd)
```

**Two kinds of actors, one shared notebook.** The N philosopher threads *do*; the main
thread *watches*. They talk to each other only through the shared `t_data` notebook, and
every page of that notebook that more than one person touches has a lock on it.

### The two structs

```c
t_data   // the shared "town hall" notice board
{
    time_start, num_of_philos, time_to_die, time_to_eat, time_to_sleep,
    num_times_philos_eat,        // -1 means "no limit"
    stop,                        // the closing-time bell
    print_lock, stop_lock,       // two mutexes
    forks,                       // array of N fork mutexes
    philo                        // array of N philosophers
}

t_philo  // one philosopher's name-tag and notebook
{
    id, meals, last_meal,
    thread,                      // their own thread handle
    meal_lock,                   // protects meals + last_meal
    lfork, rfork,                // POINTERS to the two forks they use
    data                         // pointer back to the notice board
}
```

> **Why does every philo have a `data` pointer?** A thread function receives exactly
> **one** `void *` argument. So we hand it the philosopher, and from the philosopher it
> can reach everything else (`philo->data->...`). One pointer, whole world.

> **Why are `lfork`/`rfork` pointers?** Because a fork is *shared* between two
> neighbors. If each philo had its own *copy* of a mutex, they wouldn't actually be
> sharing anything. Pointers mean "we're both pointing at the same real fork."

### The round table, in one line of code

```c
data->philo[i].lfork = &data->forks[i];
data->philo[i].rfork = &data->forks[(i + 1) % data->num_of_philos];
```

The `% num_of_philos` makes the table a **circle**: the last philosopher's right fork
wraps around to fork 0, which is also philosopher 1's left fork. (`init.c:56-57`)

---

## 4. File-by-file tour

Read them in this order. Each is short.

### `inc/philosophers.h`
Structs, includes, and the list of function prototypes. (Small tidy-ups are listed in
[section 10](#10-honest-weak-spots).)

### `src/main.c` (39 lines)
The table of contents of the whole program: init → parse → convert → init_simulation →
monitor → cleanup. On any error it writes `Error\n` to **stderr** (fd 2) and returns 1.

### `src/parse.c`: "Is the input sane?"
- `parse` → checks there are 4 or 5 arguments, then calls `validate_arg` on each.
- `validate_arg` → rejects empty strings, anything starting with `-`, and any
  non-digit. A single leading `+` is allowed.
- `convert_to_ascii` → *(the name is a bit backwards: it really goes ASCII **to**
  integer.)* Converts each string with `ft_atoi`, and `fill_data` drops each number into
  the right field *by its position* (arg 0 → number of philos, arg 1 → time to die, ...).
  `ft_atoi` returns `-1` if the number is bigger than `INT_MAX`, which we treat as an error.

### `src/utils/ft_atoi.c` and `ft_is_digit.c`
Your own tiny `atoi` and `isdigit`, since we can't use libc's. `ft_is_digit` is just "is
this character between '0' (48) and '9' (57)?"

### `src/init.c`: "Set the table"
- `init_data` → `memset` everything to zero, then set `num_times_philos_eat = -1`
  ("no meal limit unless the user gave one").
- `init_mutexes` → `malloc` the two arrays (forks and philos) and `pthread_mutex_init`
  every lock.
- `init_philos` → hands each philosopher an id (1..N), two fork pointers, a pointer to
  `data`, and sets `last_meal = time_start` (so the "starvation clock" starts at the
  beginning).
- `init_simulation` → runs those three steps, **then** `pthread_create`s the threads. If
  creating thread number `i` fails, it raises the stop flag and shrinks
  `num_of_philos` to `i` so that `cleanup` only joins threads that really exist. Clever
  detail, worth knowing!

### `src/time.c`: "The clocks"
- `now_ms` → `gettimeofday` gives seconds + microseconds. We convert:
  `seconds × 1000 + microseconds / 1000` = milliseconds.
- `is_stopped` → read the stop flag **while holding `stop_lock`**.
- `ms_sleep` → sleep for `ms` milliseconds, but in **tiny 0.5 ms naps**, re-checking the
  clock and the stop flag each time. (Why? see Q8 in [section 9](#9-questions-a-corrector-might-ask).)

### `src/print.c`: "Speaking politely"
- `set_stop` → lock, set `stop = 1`, unlock.
- `print_state` → lock `print_lock`, **only print if nobody has stopped**, unlock. Lines
  never interleave, and nothing is printed after a death.
- `print_death` → lock `print_lock`, raise the stop flag, print `died`, unlock. Because
  the stop flag goes up *while we hold the print lock*, no other message can sneak in
  after "died".

### `src/philo_routine.c`: "The philosopher's day"
See [section 5](#5-follow-one-philosopher-through-a-meal), the star of the show.

### `src/monitor.c`: "The waiter"
- `check_philo` → lock *that philosopher's* `meal_lock`; compute
  `dead = (now - last_meal > time_to_die)`; if a meal limit exists and this philo hasn't
  reached it, mark `*fed = 0`; unlock.
- `monitor` → loop forever: sweep every philosopher. Dead? Print the death and stop. If
  after the sweep `fed` is still 1 (everybody reached the limit), raise the stop flag.
  Otherwise `usleep(1000)` (1 ms) and sweep again.

### `src/cleanup.c`: "Closing time"
Join every thread (wait for them to finish), destroy every mutex, free both arrays.

---

## 5. Follow one philosopher through a meal

Let's follow **philosopher 2** at a table of 4 (`./philo 4 410 200 200`). All of this
lives in [src/philo_routine.c](src/philo_routine.c).

**Step 0: The start.** Thread is born and runs `philo_routine`.
- One philo only? → `one_philo` (see below).
- Even id (like 2)? → `ms_sleep(time_to_eat / 2)`. *The polite late arrival.* Odd
  philosophers go first; even ones come in half a meal later, so neighbors aren't
  fighting for the same forks at the same instant.

**Step 1: `eat()`.**
1. `take_forks`: even ids lock the **right** fork first, then the left. Odd ids do
   **left first, then right**. Each lock is followed by `"has taken a fork"`.
2. Lock my own `meal_lock`. Write `last_meal = now`, `meals++`. Unlock.
   *(Little lock, quick job. The monitor may be reading these exact values right now.)*
3. Print `"is eating"`.
4. `ms_sleep(time_to_eat)`: hold the forks for the whole meal.
5. Unlock both forks. The neighbors can now use them.

**Step 2: sleep.** Print `"is sleeping"`, `ms_sleep(time_to_sleep)`.

**Step 3: `think()`.** Print `"is thinking"`. If the table has an even number of
philosophers, that's it. If **odd**, also wait `2 × time_to_eat − time_to_sleep` (if
that's positive).

**Then loop** back to step 1, until `is_stopped()` says the bell has rung.

### Why does an odd table need extra thinking?

Easy way to see it: with an **odd** number of philosophers, at most `(N-1)/2` can eat at
the same time (everyone needs two forks, and one person always gets left out). It turns
out any odd table needs exactly **3 "rounds" of eating** to feed everyone once. So a full
cycle should be about `3 × time_to_eat` long. A philosopher already spends
`time_to_eat + time_to_sleep` eating and sleeping, so we pad the rest with thinking:

```
time_to_eat + time_to_sleep + think  =  3 × time_to_eat
                              think  =  2 × time_to_eat − time_to_sleep    ✓
```

That's the formula in the code (`philo_routine.c:67`). The padding keeps everyone in a
steady rhythm, so nobody gets starved by faster neighbors.

### The lonely philosopher: `one_philo`

With one philosopher there is one fork, but eating needs two. So:
lock the one fork, print `"has taken a fork"`, then wait (checking `is_stopped` every
0.5 ms) until the monitor notices they've starved and raises the stop flag. Then
unlock and exit. Expected output: two lines, `0 1 has taken a fork` and
`~800 1 died`.

---

## 6. The four kinds of lock

This is the question correctors love: *"Why do you have so many mutexes?"* Each lock
protects **one specific thing** that more than one thread touches.

| Mutex | How many | What it protects | Who touches it |
|---|---|---|---|
| `forks[i]` | N | The fork itself: only one philosopher may hold it | The two neighbors who share it |
| `meal_lock` (in each `t_philo`) | N | `last_meal` and `meals` of that philosopher | The philosopher (writes) and the monitor (reads) |
| `stop_lock` | 1 | The `stop` flag | Everyone reads it; the monitor (or setup failure) writes it |
| `print_lock` | 1 | The terminal output, so lines don't get mixed up, and nothing prints after death | Everyone who prints |

Think of it as *separate rooms with separate keys*, rather than one giant key for the
whole house. A philosopher updating their own meal time doesn't need to stop anyone
else from printing.

### The lock-order rule (the deadlock you didn't know you avoided)

There is one place where two of your locks are held **at the same time**: `print_lock`
then `stop_lock` (`print_state` → `is_stopped`, and `print_death` → `set_stop`). It is
*always* in that order. Nobody ever takes `stop_lock` first and then asks for `print_lock`. Two people
who always walk through two doors in the same order can never block each other in a
circle. If one path went `stop → print` and another `print → stop`, they could freeze
each other forever.

---

## 7. Three traps and how the code dodges them

### Trap 1: Data races
*What:* two threads access the same variable, at least one writes, and there's no lock.
The result is undefined, "anything can happen."

*Where it could happen here:* `stop` (written by monitor, read by everyone),
`last_meal` / `meals` (written by philo, read by monitor).

*Dodge:* every access to those goes through its mutex. That's the entire reason
`stop_lock` and `meal_lock` exist. `is_stopped()` is a function precisely so every
read of `stop` is done under the lock.

> **Common question: "Why not just make `stop` `volatile`?"** `volatile` only tells the
> compiler "don't cache this." It doesn't make reads and writes *safe* with respect to
> each other across threads. A mutex does both jobs.

### Trap 2: Deadlock
*What:* all N philosophers pick up their left fork. Each now waits for the right fork,
which is held by their neighbor, who is waiting for *their* right fork... a circle of
waiting, forever.

*Dodge, part 1: break the symmetry.* Odd ids take **left then right**; even ids take
**right then left**. Now the circle of "everyone holds one, wants the next" can't form,
because for every other pair of neighbors, *both* reach for their shared fork as their
first choice. One of them gets it; the other simply waits at the first step while
holding nothing, so it can't be part of a circle.

*Dodge, part 2: the polite late arrival.* Even philosophers wait `time_to_eat / 2`
before their first attempt, which also reduces the initial scramble.

### Trap 3: Starvation / dying while others hog
*What:* The program is deadlock-free, but a philosopher still doesn't get forks soon
enough.

*Dodge:* The staggered start plus the odd-table extra thinking (section 5) keep
everyone on a steady rhythm. The monitor then *measures* whether it worked: if someone
goes over `time_to_die`, it announces it.

### Bonus trap: Printing after death
Sometimes in naive solutions a philosopher prints "is eating" a tiny moment *after*
the "died" line, because the threads are racing. Here, `print_state` re-checks the stop
flag *while holding the print lock*, and `print_death` raises the flag *while holding the
print lock*. So once "died" is printed, no other line can follow. 

---

## 8. The borrowed functions cheat sheet

These are the "external functions" the project is allowed to use. A corrector might ask
what any of them does.

| Function | Plain-English meaning | Where you use it |
|---|---|---|
| `pthread_create(&t, NULL, fn, arg)` | "Hire a new worker and have them start running `fn(arg)`." Returns 0 on success | `init_simulation` |
| `pthread_join(t, NULL)` | "Wait here until worker `t` has finished." Prevents us from freeing memory they're still using | `cleanup` |
| `pthread_mutex_init(&m, NULL)` | "Make a fresh, unlocked key." | `init_mutexes` |
| `pthread_mutex_lock(&m)` | "Take the key. If someone else has it, wait in line." | everywhere |
| `pthread_mutex_unlock(&m)` | "Hand the key back." | everywhere |
| `pthread_mutex_destroy(&m)` | "Retire the key" (after everyone is done with it) | `cleanup` |
| `gettimeofday(&tv, NULL)` | "What time is it?" in seconds + microseconds | `now_ms` |
| `usleep(us)` | "Nap for about this many microseconds." Not exact! | `ms_sleep`, `monitor`, `one_philo` |
| `malloc` / `free` | Borrow / return memory | `init_mutexes` / `cleanup` |
| `memset` | Fill memory with a value (we use it to zero a struct) | `init_data` |
| `write` | Write bytes straight to a file descriptor (2 = stderr) | `main` |
| `printf` | Formatted output | `print_state`, `print_death` |

**Why `join` and not `detach`?** `detach` means "let the thread run free, I won't wait
for it." We need to *know* every thread is done before we destroy the mutexes and free
the arrays; otherwise a thread could touch freed memory. `join` gives us that guarantee.

---

## 9. Questions a corrector might ask

Say these in your own words. The answers here are just a starting point.

**Q1. Walk me through your program.**
Use the 60-second version in section 1, then offer to open any file.

**Q2. What is a thread? What's the difference from a process?**
A thread is like another person working in the *same kitchen*, sharing the same
ingredients (memory). A process is a separate kitchen with its own ingredients. Threads
are cheaper to start and share memory directly, which is both the benefit and the danger
(data races).

**Q3. What is a mutex and why do you need one?**
A mutex is a key that only one thread can hold. We need it any time two threads might
use the same data or resource at once: forks, the stop flag, meal times and printing.

**Q4. What is a data race? Show me where one would happen if you removed a lock.**
Two threads using the same memory at once, with at least one writing and no lock.
Example: remove `meal_lock` and the monitor could read `last_meal` half-way through a
philosopher writing it, and wrongly declare a death (or miss one).

**Q5. How do you prevent deadlock?**
By not letting the "everyone holds one fork and waits for the other" circle form: odd
ids take left-then-right, even ids take right-then-left, and even ids start half a meal
late.

**Q6. What happens with one philosopher?**
There's one fork and eating needs two. The philosopher takes the one fork, prints it, and
waits. The monitor sees they've gone past `time_to_die` and prints `died`. (`one_philo`
exists so we never try to lock the same fork twice, which would freeze the thread.)

**Q7. How does the simulation know when to stop?**
Two ways, both handled by the monitor: (a) someone's `now − last_meal > time_to_die` →
print `died`; (b) if a meal count was given and every philosopher has eaten at least that
many times → stop quietly. Both raise `data->stop`, and every loop checks it.

**Q8. Why not just `usleep(time_to_eat * 1000)`?**
Two reasons. (1) `usleep` can *oversleep*, and small errors pile up over many meals; the
death timing needs to be accurate to within a few ms. (2) A long `usleep` can't be
interrupted, so after a death the threads would keep napping before they notice. Our
`ms_sleep` naps in 0.5 ms slices and re-checks the clock and the stop flag each time.

**Q9. How precise is your death detection?**
The monitor sweeps every 1 ms, so a death is reported within roughly 1-2 ms of the
deadline. The subject asks for within 10 ms. In my tests: `./philo 1 800 200 200` printed
`801 1 died`; `./philo 4 310 200 100` printed `311`/`312 ... died`.

**Q10. Why is `stop` protected by a mutex? It's just an `int`.**
Reading while another thread writes is still a data race, even for an `int`. The mutex
makes the read/write safe *and* makes sure each thread sees the latest value instead of
a stale copy.

**Q11. Why is printing locked?**
So lines from different threads can't get mixed together, and so nothing prints after
the death message (we check the stop flag while holding the print lock).

**Q12. Why does the monitor lock each philosopher's `meal_lock`?**
`last_meal` and `meals` are written by the philosopher thread and read by the monitor at
the same time. The lock keeps those reads and writes from colliding.

**Q13. Why do even philosophers wait at the start?**
So neighbors aren't all reaching for forks at the same instant. It spreads out the
first meals, which keeps things smooth and avoids an early scramble.

**Q14. Why the extra `think` time with an odd number?**
With an odd table, someone is always left out each round, and a full cycle takes ~3
meal-times. Thinking `2×time_to_eat − time_to_sleep` pads each philosopher's cycle to
match, so nobody is starved.

**Q15. What happens if `pthread_create` fails halfway?**
`init_simulation` raises the stop flag (so already-started threads exit) and sets
`num_of_philos = i`, so `cleanup` only joins the `i` threads that were actually created.
It then returns an error and `main` prints `Error`.

**Q16. How do you check for races and leaks?**
Build with `make re SAN=thread` (ThreadSanitizer: finds data races) or
`make re SAN=address` (AddressSanitizer: memory errors). On Linux also `valgrind
--leak-check=full` and `valgrind --tool=helgrind`. See [section 12](#12-commands-to-try-yourself-tonight).

**Q17. Why does `philo_routine` take and return `void *`?**
That's the required signature for any function given to `pthread_create`: one `void *`
argument in, one `void *` out. We cast the argument back to `t_philo *` on the first line.

**Q18. What does `% num_of_philos` do in `init_philos`?**
Makes the table a circle: the last philosopher's right fork wraps around to fork 0.

**Q19. Why do you pass `&data->philo[i]` to each thread and not `data`?**
Each thread needs to know *which* philosopher it is (id, forks, meal info). `philo->data`
still gives it access to the shared data.

**Q20. What's the purpose of `last_meal = time_start` at init?**
So the "hunger clock" of every philosopher starts at the beginning of the simulation. If
they don't start eating within `time_to_die`, they die.

---

## 10. Honest weak spots

These aren't bad news. They're a heads-up so you can answer calmly instead of being
caught off guard. I **did not change your code** for any of these.

1. **`0` is accepted.** `./philo 0 800 200 200` exits with code 0 and prints nothing.
   Other zeros (like `time_to_die = 0`) are accepted too. Some correctors consider "0
   philosophers" an invalid input that should print an error. *If asked:* "I treat the
   arguments as valid unsigned integers, 0 philosophers means no simulation." Or, if
   you'd like, it's a one-line check in `parse` to reject it, and I can add that for you.
2. **`./philo 5 800 200 200 0`** (must eat 0 times) prints a few lines and stops
   immediately, because the goal is already satisfied. That's logical, and not covered by
   the subject.
3. **Meals are counted when eating *starts***, not when it ends (`meals++` happens before
   `ms_sleep(time_to_eat)`). So the simulation can stop while the last philosopher is
   mid-meal. It's fine for the subject ("has eaten at least N times"), but know the detail.
4. **If setup fails halfway** (e.g. `malloc` or `pthread_mutex_init` fails midway),
   `cleanup` might try to join or destroy something that was never created. This is a
   very rare path and not something evaluators normally trigger.
5. **A misleading comment:** above `take_forks` the comment says *"odd philos need extra
   thinking to give others time to eat"*. That's really about the **think** step. The
   function itself is about fork **order**. Don't repeat that comment, describe what the
   code does.
6. **`convert_to_ascii`'s name** is backwards: it converts ASCII text **to** an integer
   (like `atoi`). If asked, just explain what it actually does.
7. **Norminette:** it isn't installed on this machine, so I couldn't run it. Please
   run it at school. Things I noticed that it may flag:
   - 4-space indentation instead of tabs in `parse.c`, `monitor.c`, `print.c`, parts of
     `time.c`, `ft_atoi.c`, `ft_is_digit.c`.
   - A **variable-length array** (`int args[argc - 1];`) in `convert_to_ascii`.
   - `//` comments **inside** function bodies (`// case for 1`, `//verif`).
   - Commented-out code in `ft_is_digit.c`.
   - Functions `init_philos` and `check_philo` aren't in the header (and aren't `static`),
     and `time.c`'s prototypes are listed twice in the header.
   - An unused `#include <semaphore.h>` with the comment "IDK".
8. **Your numbered comments** (`// 1. main`, `// 1.5 init_data`, `// 2. parse` ... `// 9.
   philo_routine`) are actually a handy map of **run order**: main (1) → init_data (1.5)
   → parse (2) → validate_arg (3) → ft_is_digit (4) → convert_to_ascii (5) → ft_atoi (6)
   → fill_data (7) → init_simulation (8) → init_mutexes (8.1) → init_philos (8.3) →
   philo_routine (9). Not every
   function has a number yet, so if your last commit message ("needs numbers on
   everything") meant that, section 3's diagram can serve as the master order.

---

## 11. What I actually tested

I built a clean copy (so your repo stayed untouched) and ran everything on **macOS**.
Not tested: Linux, valgrind/helgrind, norminette (none are on this machine).

| Command | Expected | Result |
|---|---|---|
| `./philo 1 800 200 200` | takes a fork, dies at ~800 | `0 1 has taken a fork` / `801 1 died` ✅ |
| `./philo 5 800 200 200` | nobody dies (runs forever) | ran 8 s, no death ✅ |
| `./philo 5 800 200 200 7` | stops once all ate ≥ 7 times | stopped, no death ✅ |
| `./philo 4 410 200 200` | nobody dies | ran 8 s, no death ✅ |
| `./philo 3 600 200 200 5` | odd table, stops after meals | stopped, no death ✅ |
| `./philo 5 610 200 200 6` | tight odd table | stopped, no death ✅ |
| `./philo 4 310 200 100` | someone dies | `311`/`312 ... died` ✅ |
| `./philo 4 200 205 200` | someone dies | `201 1 died` ✅ |
| `./philo 200 800 200 200 2` | big table | finished, no death ✅ |
| `./philo 31 610 200 100 4` | big odd table | finished, no death ✅ |
| `./philo abc 800 200 200`, `-5 ...`, missing arg, `2147483648` | `Error` | `Error` on stderr, exit 1 ✅ |
| ThreadSanitizer (`SAN=thread`) on 5 scenarios | no race reports | none ✅ |
| AddressSanitizer (`SAN=address`) on 5 scenarios | no memory errors | none ✅ |
| macOS `leaks --atExit` | 0 leaks | 0 leaks ✅ |

Tip: if you redirect output to a file (`> out.txt`), `printf` is buffered and you may see
fewer lines if you kill the program. That's normal and not a bug.

---

## 12. Commands to try yourself tonight

Build:

```bash
make
```

Watch a happy table (no one should die; press Ctrl+C to stop):

```bash
./philo 5 800 200 200
```

Watch one that stops by itself after everyone ate 5 times:

```bash
./philo 5 800 200 200 5
```

Watch a lonely philosopher die:

```bash
./philo 1 800 200 200
```

Watch a doomed table (someone should die near 310 ms):

```bash
./philo 4 310 200 100
```

Count that each philosopher ate the right number of times (nice trick for a live demo):

```bash
./philo 5 800 200 200 7 | grep eating | awk '{print $2}' | sort | uniq -c
```

Hunt for data races (rebuild with the thread sanitizer):

```bash
make fclean && make re SAN=thread && ./philo 5 800 200 200 4
```

Hunt for memory problems:

```bash
make fclean && make re SAN=address && ./philo 5 800 200 200 4
```

On Linux at school, also try:

```bash
valgrind --leak-check=full ./philo 5 800 200 200 4
valgrind --tool=helgrind ./philo 5 800 200 200 4
```

Go back to a normal build when you're done:

```bash
make fclean && make
```

(Do not mix `SAN=thread` and `SAN=address`; always `fclean` between them. Your Makefile
says so too.)

---

## 13. Self-quiz

Cover the answers with your hand (or just click to expand them).

<details>
<summary>1. What do the threads represent, and what do the mutexes represent?</summary>

Threads are the philosophers; fork mutexes are the forks. There are also mutexes that
guard shared data: `stop_lock`, `print_lock`, and each philosopher's `meal_lock`.
</details>

<details>
<summary>2. Name the three arguments that are times, and say which one sets the deadline for dying.</summary>

`time_to_die`, `time_to_eat`, `time_to_sleep` (all in ms). `time_to_die` is the deadline:
time since the start of the last meal (or the start of the simulation).
</details>

<details>
<summary>3. What does <code>num_times_philos_eat = -1</code> mean?</summary>

"No meal limit." The simulation only ends if somebody dies.
</details>

<details>
<summary>4. Which thread runs <code>monitor()</code>?</summary>

The main thread, after it has launched all the philosopher threads.
</details>

<details>
<summary>5. Which lock is always taken first when two are held together?</summary>

`print_lock`, then `stop_lock`. Never the reverse.
</details>

<details>
<summary>6. Why does a philosopher take <code>meal_lock</code> only for two lines of code?</summary>

Locks should be held as briefly as possible; the lock only needs to cover updating
`last_meal` and `meals`. Holding it during the whole meal would make the monitor wait.
</details>

<details>
<summary>7. If you deleted <code>take_forks</code>'s odd/even logic, what could go wrong?</summary>

Everyone might take their left fork at once, then all wait forever for the right:
a deadlock.
</details>

<details>
<summary>8. In <code>ms_sleep</code>, why check <code>is_stopped</code> inside the loop?</summary>

So a sleeping philosopher wakes up almost immediately once the simulation ends, instead
of finishing a long nap.
</details>

<details>
<summary>9. Why is there a special case for exactly one philosopher?</summary>

There's only one fork, so the normal "take two forks" code would try to lock the same
fork twice and hang. `one_philo` takes it, waits, and lets the monitor declare the death.
</details>

<details>
<summary>10. What are the three things <code>cleanup</code> does, in order?</summary>

Join all threads, destroy all mutexes, free the arrays.
</details>

---

You've got this. A corrector isn't looking for perfection; they're looking for
understanding, and you now have a clear story to tell. If a question stumps you, it's
completely fine to say, "Let me look at the code," and then trace it using section 3.
That is what real programmers do.

Good luck tonight and tomorrow. 🍝
