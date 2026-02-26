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

const int MAX_THREADS = std::thread::hardware_concurrency() / 2;

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
      lock.unlock();
      active_tasks++;
      task();
      active_tasks--;
      cv.notify_all();
    }
  };
public:
  ThreadPool(std::size_t threads_num) {
    if (threads_num > MAX_THREADS) {
      threads_num = MAX_THREADS;
      std::cout << "Thread pool size is too large, set to " << MAX_THREADS << std::endl;
    }

    this->workers.reserve(threads_num);
    for (size_t i = 0; i < threads_num; i++) {
      this->workers.push_back(std::thread(&ThreadPool::worker_thread, this));
    }
  };
  
  ~ThreadPool(){
    this->stop = true;
    this->cv.notify_all();
    for (auto& worker : workers){
      worker.join();
    }
  }

  void push_task(std::function<void()> task){
    {
      std::lock_guard<std::mutex> lock(mtx);
      tasks.push(task);
    }
    cv.notify_one();
  };

  void wait(){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]{return active_tasks == 0 && tasks.empty();});
  };
};  

int main() {
  ThreadPool pool(MAX_THREADS);
  std::cout << "ThreadPool created with " << MAX_THREADS << " threads." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < 1000; i++) {
    pool.push_task([](){
      // std::this_thread::sleep_for(std::chrono::milliseconds(100));
      volatile long long sum = 0;
      for (int i = 0; i < 1000000; i++) sum += i;
    });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  pool.wait();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "Run time: " << diff.count() << " seconds" << std::endl;


  auto start2 = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < 1000; i++) {
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    volatile long long sum = 0;
    for (int i = 0; i < 1000000; i++) sum += i;
  }

  auto end2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff2 = end2 - start2;
  std::cout << "Run time: " << diff2.count() << " seconds" << std::endl;

  return 0;
}
