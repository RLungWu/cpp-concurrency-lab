# 01-thread-pool

## Goals
- Implement a thread pool from scratch
- Understand mutex, condition_variable, and atomic
- Measure and explain performance differences
- Explore work stealing as a solution to contention

## Roadmap
- [ ] Week 1 — Minimal thread pool (compile, run, works)
- [ ] Week 2 — Benchmarking + observing contention
- [ ] Week 3–4 — Work stealing / per-thread queues

## Build
```bash
g++ -std=c++17 -pthread main.cpp -o threadpool
./threadpool
```

## What I'm learning
Not just how to use a thread pool —
but why contention kills performance, and how design decisions change that.