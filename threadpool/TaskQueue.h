//
// Created by Rambo.Liu on 2023/5/18.
//

#ifndef M3U8_TASKQUEUE_H
#define M3U8_TASKQUEUE_H

#include <pthread.h>
#include <stdio.h>
#include <queue>


/**
 * 定义任务执行的结构体，由具体任务去执行
 */
using callback = void (*)(void *);

struct Task {
    Task() {
        std::cout << "Task 构造 " << this << std::endl;
        function = nullptr;
        arg = nullptr;
    }
    ~Task() {
        std::cout << "Task 析构 " << this << std::endl;
    }
    /**
     * 拷贝构造函数
     *
     * @param task
     */
    Task(const Task& task) {
        function = task.function;
        arg = task.arg;
        std::cout << "Task 拷贝构造 " << this << std::endl;
    }

    Task(callback f, void *arg) {
        function = f;
        this->arg = arg;
    }
    callback function{};
    void *arg{};  //回调待执行的参数
};

/**
 * 任务队列
 */
class TaskQueue {

public:

    TaskQueue();

    ~TaskQueue();

    /**
     * 添加任务
     * @param task
     */
    void addTask(Task &task);

    /**
     * 添加任务
     *
     * @param func
     * @param arg
     */
    void addTask(callback func, void *arg);

    /**
     * 弹出任务 执行
     * @return
     */
    Task takeTask();

    /**
     * 获取当前队列中任务个数
     * @return
     */
    int taskNumber();


    pthread_mutex_t m_mutex;    // 互斥锁
    std::queue<Task> m_queue;   // 任务队列

    int m_taskCount;
};


#endif //M3U8_TASKQUEUE_H
