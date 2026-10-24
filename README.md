# Quant

A focused C++ learning and experimentation repository covering low-level concurrency concepts and order book mechanics used in quantitative trading systems.

## Overview

This project is organized into two main areas:

- C++ language and concurrency concepts
- Trading order book implementations and matching logic

The repository is intended for learning, prototyping, and reinforcing practical understanding of thread safety, atomic operations, and market microstructure.

## Repository Structure

```text
Quant/
├── Cpp_concepts/
│   ├── Atomic_operation/
│   ├── Casting/
│   ├── Threading/
│   └── lvalue_&&_rvalue.cpp
├── OrderBook_v1/
│   ├── orderbook.cpp
│   ├── orderbook.h
│   ├── orderbook_test_basic.cpp
│   └── CMakeLists.txt
├── Order_Book_v2/
│   ├── Orderbook.h
│   ├── Orderbook.cpp
│   ├── Order.h
│   ├── Trade.h
│   └── ...
├── hackerrank-orchestrater/
├── build/
└── README.md
```

## C++ Concepts

The `Cpp_concepts` directory contains practical examples and notes on common C++ features, with emphasis on correctness in multithreaded environments.

### Atomic operations

Examples include:

- `Atomic_flag.cpp` — demonstrates lock acquisition using `std::atomic_flag`
- `Atomic_bool.cpp` — explores boolean atomic semantics
- `Atomic_pointer.cpp` — pointer-based atomic operations
- `Compare_exchange.cpp` — compare-and-swap patterns
- `memory_order_acquire_and_release.cpp` — release/acquire synchronization
- `memory_order_relaxed.cpp` and `memory_order_sequential.cpp` — memory ordering behavior

These examples highlight how atomic variables and memory ordering are used to coordinate threads without relying on unsafe shared state.

### Threading and synchronization

This section includes small examples related to thread creation, synchronization, and concurrency patterns. It is useful for understanding how data races and locking rules affect software correctness.

### Type conversions and language details

The `Casting` folder and the standalone value-category example (`lvalue_&&_rvalue.cpp`) provide a practical introduction to:

- `static_cast`
- `dynamic_cast`
- `reinterpret_cast`
- `const_cast`
- lvalue and rvalue semantics

## Order Book Projects

The order book implementations model a simplified matching engine for trading systems.

### OrderBook_v1

This version includes:

- a basic order book implementation
- matching logic for trade execution
- Google Test validation
- CMake-based build configuration

The project is a good example of how a simple execution engine can be structured for testing and iteration.

### Order_Book_v2

This directory is a more advanced iteration with additional trading abstractions and order-level metadata, including structures for:

- order types
- side-specific book management
- trade records
- level information
- order modification and validation logic

It reflects the complexity of real order-book processing in financial markets.

## Getting Started

### Prerequisites

- C++17 or newer
- CMake (for the order book v1 project)
- A C++ compiler such as `g++` or `clang++`
- Optional: Google Test for the v1 test target

### Build and run a C++ concept example

```bash
g++ -std=c++17 -pthread Cpp_concepts/Atomic_operation/Atomic_flag.cpp -o atomic_flag
./atomic_flag
```

### Build OrderBook_v1

```bash
cd OrderBook_v1
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Learning Focus

This repository is designed to strengthen understanding of:

- thread safety and synchronization
- atomic operations and memory ordering
- order matching mechanics
- data structure design for trading systems
- testing and validation in C++ projects

## Notes

The codebase is intentionally educational and practical rather than production-ready. It is best used as a reference for understanding core principles behind concurrent systems and execution engines.

## License

This project does not currently declare a specific license. If you intend to reuse or redistribute the code, confirm the licensing requirements before publication or commercial use.
