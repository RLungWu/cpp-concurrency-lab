#include <condition_variable>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

const int MAX_THREADS = 10;

class ThreadPool {
private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex mtx;
  std::condition_variable cv;
  bool stop = false;

public:
  ThreadPool(std::size_t threads_num) {
    if (threads_num > MAX_THREADS) {
      threads_num = MAX_THREADS;
      std::cout << "Thread pool size is too large, set to " << MAX_THREADS << std::endl;
    }

    this->workers.reserve(threads_num);
    for (size_t i = 0; i < threads_num; i++) {
      this->workers.push_back(std::thread());
    }
  };

  void push_task(std::function<void()> task){
    this->tasks.push(task);  
    this->cv.notify_one();
  };
};  

int main() {
  ThreadPool pool(4);
  std::cout << "ThreadPool created with 4 threads." << std::endl;
  return 0;
}
