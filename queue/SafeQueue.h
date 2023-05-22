//
// Created by Rambo.Liu on 2023/5/17.
// 线程安全的队列 参考 https://wangpengcheng.github.io/2019/05/17/cplusplus_theadpool/

#ifndef M3U8_SAFEQUEUE_H
#define M3U8_SAFEQUEUE_H

#include <mutex>
#include <queue>

template<typename T>
class SafeQueue {
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;

public:
    SafeQueue() {

    }

    //拷贝构造函数
    SafeQueue(SafeQueue &other) {

    }

    ~SafeQueue() {

    }

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }

    bool dequeue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front()); //取出队首元素，返回队首元素值，并进行右值引用
        m_queue.pop();
        return true;
    }
    void clear() {
        std::unique_lock<std::mutex> lock(m_mutex);
        int size = m_queue.size();
        while(!m_queue.empty()) {
            T value = m_queue.front();
            delete value;
            value = nullptr;
            m_queue.pop();
        }
    }
};


#endif //M3U8_SAFEQUEUE_H
