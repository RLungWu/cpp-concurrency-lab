# 01-thread-pool

## Goals
- Implement a thread pool from scratch
- Understand mutex, condition_variable, and atomic
- Measure and explain performance differences
- Explore work stealing as a solution to contention

## Benchmark Results -- Test on AMD Ryzen 7 9700X 8-Core Processor
### Same Threads, Different Workloads

| Threads | Tasks | Workload | Throughput | Median latency |
|---------|-------|----------|------------|----------------|
| 4 | 1,000 | 100 | 1,067,050 tasks/sec | 0.000937164 s |
| 4 | 1,000 | 1,000 | **1,439,300 tasks/sec** | 0.000694784 s |
| 4 | 1,000 | 10,000 | 698,568 tasks/sec | 0.0014315 s |
| 4 | 1,000 | 100,000 | 96,734.2 tasks/sec | 0.0103376 s |
| 4 | 1,000 | 1,000,000 | 9,804.85 tasks/sec | 0.10199 s |

**Weird Point: workload=100 is slower than workload=1000.**
This is the direct evidence of mutex contention. When task is too light, threads waste too much time to compete for mutex lock but doing the job.


### Varying Threads (workload = 1,000)

| Threads | Tasks | Workload | Throughput | Median latency |
|---------|-------|----------|------------|----------------|
| 1 | 1,000 | 1,000 | 1,478,310 tasks/sec | 0.00067645 s |
| 2 | 1,000 | 1,000 | **2,852,300 tasks/sec** | 0.000350594 s |
| 4 | 1,000 | 1,000 | 1,635,090 tasks/sec | 0.000611588 s |
| 8 | 1,000 | 1,000 | 477,509 tasks/sec | 0.0020942 s |
| 16 | 1,000 | 1,000 | 270,219 tasks/sec | 0.0037007 s |
| 32 | 1,000 | 1,000 | 289,470 tasks/sec | 0.00345459 s |

**Weird Point: Thread=2 is the max throughput, but the Thread=4 is slower.**
By observing, the throughput is linear growing from thread=1 to thread=2. However, thread=4 suddenly decrease, that's the mutex contention.
The other interesting thing is that thread=16 and thread=32, the throughput of 32 is higher than thread=16. This means that in high contention situation, high threads numbers make OS scheduler execute concurrency, decrease the waiting time. 

## Build
```bash
g++ -std=c++17 -pthread main.cpp -o threadpool
./threadpool
```

## What I'm learning
Not just how to use a thread pool —
but why contention kills performance, and how design decisions change that.