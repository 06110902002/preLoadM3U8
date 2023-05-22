//
// Created by Rambo.Liu on 2023/5/18.
//

#include <iostream>
#include "TaskQueue.h"


TaskQueue::TaskQueue() {
    m_taskCount = 0 ;
    pthread_mutex_init(&m_mutex, nullptr);
}

TaskQueue::~TaskQueue() {
    pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task &task) {
    std::cout<< "TaskQueue::addTask 添加任务 "<< &task <<std::endl;
    pthread_mutex_lock(&m_mutex);
    m_queue.push(task);
    m_taskCount ++;
    pthread_mutex_unlock(&m_mutex);
    std::cout<< "添加任务 结束 当前任务 个数 = "<<m_taskCount << std::endl;
}

void TaskQueue::addTask(callback func, void *arg) {
    pthread_mutex_lock(&m_mutex);
    Task task;
    task.function = func;
    task.arg = arg;
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::takeTask() {
    Task t;
    std::cout<<"Task TaskQueue::takeTask() " << &t <<std::endl;
    pthread_mutex_lock(&m_mutex);
    if (!m_queue.empty()) {
        t = m_queue.front();
        m_queue.pop();
        m_taskCount --;
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}

int TaskQueue::taskNumber() {
    int taskSize = 0;
    pthread_mutex_lock(&m_mutex);
    taskSize =  m_taskCount;
    pthread_mutex_unlock(&m_mutex);
    return taskSize;
}
