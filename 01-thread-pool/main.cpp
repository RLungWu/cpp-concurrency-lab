#include <condition_variable>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

const size_t MAX_THREADS = std::max(1u, std::thread::hardware_concurrency());

class ThreadPool {
private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex mtx;
  std::condition_variable cv;
  bool stop = false;
  std::atomic<size_t> active_tasks{0};
  
  void worker_thread(){
    while(true){
      std::unique_lock<std::mutex> lock(mtx);  
      cv.wait(lock, [this]{return !tasks.empty() || stop;});
      if (stop && tasks.empty()) return;
      auto task = tasks.front();
      tasks.pop();
      active_tasks++;
      lock.unlock();
      task();
      active_tasks--;
      cv.notify_all();
    }
  };
public:
  ThreadPool(std::size_t threads_num) {
    this->workers.reserve(threads_num);
    for (size_t i = 0; i < threads_num; i++) {
      this->workers.push_back(std::thread(&ThreadPool::worker_thread, this));
    }
  };
  
  ~ThreadPool(){
    // this->stop = true;
    {
      std::lock_guard<std::mutex> lock(mtx);
      stop = true;
    }
    cv.notify_all();
    for (auto& worker : workers){
      worker.join();
    }
  }

  void push_task(std::function<void()> task){
    {
      std::lock_guard<std::mutex> lock(mtx);
      if (stop) throw std::runtime_error("ThreadPool is stopped");
      tasks.push(std::move(task));
    }
    cv.notify_one();
  };

  void wait(){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]{return active_tasks == 0 && tasks.empty();});
  };
}; 

double benchmark(std::function<void()> fn, int warmup = 3, int runs = 10) {
    // 1. Warm-up：讓 cache 和 thread 進入穩定狀態
    for (int i = 0; i < warmup; i++) fn();

    // 2. 多次測量
    std::vector<double> results;
    for (int i = 0; i < runs; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        fn();
        auto end = std::chrono::high_resolution_clock::now();
        results.push_back(std::chrono::duration<double>(end - start).count());
    }

    // 3. 取中位數（比平均值更穩定，不受極端值影響）
    std::sort(results.begin(), results.end());
    return results[runs / 2];
}

void run_benchmark(int num_threads, int num_tasks, int task_workload) {
    ThreadPool pool(num_threads);

    auto fn = [&]() {
        for (int i = 0; i < num_tasks; i++) {
            pool.push_task([task_workload]() {
                volatile long long sum = 0;
                for (int i = 0; i < task_workload; i++) sum += i;
            });
        }
        pool.wait();
    };

    double median_time = benchmark(fn);
    double throughput = num_tasks / median_time;

    std::cout << "threads=" << num_threads
              << " tasks=" << num_tasks
              << " workload=" << task_workload
              << " → " << throughput << " tasks/sec"
              << " (median: " << median_time << "s)\n";
}

int main() {
  for (int workload: {100, 1000, 10000, 100000, 1000000}){
    run_benchmark(4, 1000, workload);
  }

  for (int threads: {1, 2, 4, 8, 16, 32}){
    run_benchmark(threads, 1000, 1000);
  }

  return 0;
}
