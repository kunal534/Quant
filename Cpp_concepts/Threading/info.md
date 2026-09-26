# Threading

- If no thread is made, it is a single-threaded process and it starts from `int main()`.
- `Thread`: A lightweight process.
- Examples of it's used:
  - Different tabs are being used as different threads.
  - Intellicense uses threads.
## There are different ways of creating it:

```
  1. Function Pointers
  2. Lambda Functions
  3. Functors
  4. Member Functions
  5. Static Member functions 
```

- To complite it we need to write it in following format: 
  ```
    g++ -std=c++11 -pthread [file name]
  ```
  meanings: pthread -> POSIX( Portable Operatin System Interface ) threads is a standard API ( between our program and OS ) that allows a program to create, control and synchronize multiple threads of execution 

  - syntax 
  std::threadt [name](function name , parameters,,);
   note: need to join it with main thread else it would keep running in background