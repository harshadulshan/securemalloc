\# SecureMalloc



A production-grade secure memory allocator built from scratch in C,

with a live C# dashboard and SQLite profiler backend.



\## What It Does



SecureMalloc replaces the standard malloc/free with a security-hardened

allocator that detects memory vulnerabilities in real time.



\## Security Features



\- Buffer overflow detection via canary values

\- Use-after-free detection via poison memory patterns

\- Double free detection

\- Null pointer protection

\- Memory leak detection with exact file and line number

\- Malloc hook — intercepts all malloc/free calls automatically



\## Project Structure



securemalloc/

├── include/

│   └── securemalloc.h        # Core API and data structures

├── src/

│   ├── securemalloc.c        # Allocator + security engine

│   ├── malloc\_hook.c         # malloc/free/calloc/realloc override

│   ├── db\_logger.c           # SQLite allocation logger

│   └── sqlite3.c             # SQLite amalgamation

├── tests/

│   ├── test\_alloc.c          # Allocator tests

│   ├── test\_db.c             # Database logger tests

│   └── test\_hook.c           # Malloc hook tests

├── SecureMalloc.Dashboard/   # C# live dashboard

│   └── Program.cs

└── securemalloc.db           # Allocation history database



\## How To Build



\### Requirements

\- GCC 13+ for Windows (MinGW-w64)

\- .NET 10 SDK

\- SQLite amalgamation (sqlite3.h + sqlite3.c)



\### Build Core Allocator

```bash

cd tests

gcc test\_alloc.c ..\\src\\securemalloc.c -o test\_alloc.exe -I..\\include

.\\test\_alloc.exe

```



\### Build Database Logger

```bash

gcc test\_db.c ..\\src\\securemalloc.c ..\\src\\db\_logger.c ..\\src\\sqlite3.c -o test\_db.exe -I..\\include -I..\\src

.\\test\_db.exe

```



\### Build Malloc Hook

```bash

gcc test\_hook.c ..\\src\\securemalloc.c ..\\src\\malloc\_hook.c ..\\src\\db\_logger.c ..\\src\\sqlite3.c -o test\_hook.exe -I..\\include -I..\\src

.\\test\_hook.exe

```



\### Run Dashboard

```bash

cd SecureMalloc.Dashboard

dotnet run

```



\## Dashboard Features



\- Live allocation summary with color coding

\- Critical vs warning severity levels

\- ASCII allocation graph over time

\- Auto refresh every 5 seconds

\- CSV export of full allocation history



\## Algorithms Used



\- Canary generation and validation

\- Poison memory patterns (0xCD alloc, 0xDD free)

\- Doubly linked list heap tracking

\- Hash based leak detection

\- SQLite query based allocation profiling



\## Technologies



| Layer      | Technology        |

|------------|-------------------|

| Allocator  | C (GCC)           |

| Security   | C (canary/poison) |

| Database   | SQLite            |

| Dashboard  | C# .NET 10        |

| Profiler   | C + SQLite        |



\## What This Demonstrates



\- Deep understanding of how memory actually works

\- How heap exploits happen and how to prevent them

\- System level programming in C

\- Cross language integration C and C#

\- Real time profiling and monitoring



\## Author



Built as a learning project covering:

system programming, security, algorithms, and database integration.



