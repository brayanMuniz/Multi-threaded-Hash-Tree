# Multi-threaded-Hash-Tree

## Project Overview

This project implements a **multi-threaded hash tree** algorithm that computes a hash of a memory-mapped file by breaking it into blocks and processing each block using multiple threads. The program utilizes the Jenkins one-at-a-time hash function and a binary tree structure to compute and combine hash values, improving performance through parallel processing.

## Features
- **Multi-threaded processing**: Efficiently splits a file into blocks and processes each part concurrently using multiple threads.
- **Memory-mapped file access**: Uses `mmap` to minimize I/O overhead, mapping large files directly into memory.
- **Jenkins one-at-a-time hash**: A simple, non-cryptographic hash function optimized for speed.
- **Threaded hash calculation**: Each thread computes a hash for a portion of the file, and parent threads combine these hashes using a tree structure.
- **Dynamic thread allocation**: Supports specifying the number of threads to optimize performance based on available system resources.

## Requirements
- **C compiler**: GCC or equivalent.
- **Libraries**: The code relies on standard libraries like `pthread`, `mmap`, and `sys/stat.h` for multi-threading and memory-mapped I/O.
- **Hardware**: A multi-core CPU is recommended to take full advantage of the multi-threading capabilities.

## Performance Analysis

Based on the experimental results (detailed in the report), the program exhibits significant speedup when increasing the number of threads, with optimal performance at 64 threads on a 48-core CPU. The performance gains diminish when using more than 64 threads due to thread management overhead and system limitations. Results for different thread counts are as follows:
