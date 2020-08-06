#ifndef MPRPC_SRC_INCLUDE_LOCKQUEUE_H_
#define MPRPC_SRC_INCLUDE_LOCKQUEUE_H_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// log queue asynchronic write
template<typename T>
class LockQueue {
public:
    // multi-threads write log queue
    void Push(const T& val) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(val);
        condition_var_.notify_all();
    }

    // only one thread read log queue
    T Pop() {
        std::unique_lock<std::mutex> lock_ptr(mutex_);
        while (queue_.empty()) {
            // log queue empty, thread wait
            condition_var_.wait(lock_ptr);
        }

        T val = queue_.front();
        queue_.pop();
        return val;
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condition_var_;
};

#endif // MPRPC_SRC_INCLUDE_LOCKQUEUE_H_