//
// Created by Rambo.Liu on 2023/5/18.
//

#include <unistd.h>
#include "ThreadPool.h"

//using namespace std;

ThreadPool::ThreadPool(int minNum, int maxNum) {
    m_taskQ = new TaskQueue;
    do {
        m_minNum = minNum;
        m_maxNum = maxNum;
        m_busyNum = 0;
        m_aliveNum = minNum;

        m_threadIDs = new pthread_t[maxNum];
        if (m_threadIDs == nullptr) {
            std::cout << "malloc thread_t[] 失败...." << std::endl;;
            break;
        }
        memset(m_threadIDs, 0, sizeof(pthread_t) * maxNum);
        if (pthread_mutex_init(&m_lock, NULL) != 0 ||
            pthread_cond_init(&m_notEmpty, NULL) != 0) {
            std::cout << "init mutex or condition fail..." << std::endl;
            break;
        }

        // 根据最小线程个数, 创建任务线程
        for (int i = 0; i < minNum; ++i) {
            pthread_create(&m_threadIDs[i], nullptr, worker, this);
            std::cout << "创建子线程, ID: " << m_threadIDs[i] << std::endl;
        }
        // 创建管理者线程, 1个
        pthread_create(&m_managerID, nullptr, manager, this);

    } while (0);
}

ThreadPool::~ThreadPool() {
    shutDown();
    std::cout << "ThreadPool 线程池析构" << std::endl;
}

void ThreadPool::addTask(Task& task) {
    if (m_shutdown) {
        return;
    }
    std::cout<<"ThreadPool::addTask 添加任务 "<< &task <<std::endl;
    m_taskQ->addTask(task);
    pthread_cond_signal(&m_notEmpty);
}

int ThreadPool::getAliveNumber() {
    int threadNum = 0;
    pthread_mutex_lock(&m_lock);
    threadNum = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

int ThreadPool::getBusyNumber() {
    int busyNum = 0;
    pthread_mutex_lock(&m_lock);
    busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}


// 工作线程任务函数
void *ThreadPool::worker(void *arg) {
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    while (true) {
        // 访问任务队列(共享资源)加锁
        pthread_mutex_lock(&pool->m_lock);
        // 判断任务队列是否为空, 如果为空工作线程阻塞
        std::cout << "当前任务个数 = " << pool->m_taskQ->taskNumber() << std::endl;
        while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown) {
            std::cout << "thread " << pthread_self() << " waiting..." << std::endl;
            // 阻塞线程
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);

            // 解除阻塞之后, 判断是否要销毁线程
            if (pool->m_exitNum > 0) {
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum) {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }
        // 判断线程池是否被关闭了
        if (pool->m_shutdown) {
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }
        std::cout << "thread " << pthread_self() << " 被唤醒..." << std::endl;
        // 从任务队列中取出一个任务
        Task task = pool->m_taskQ->takeTask();
        pool->m_busyNum++;
        pthread_mutex_unlock(&pool->m_lock);
        // 执行任务
        std::cout << "thread " << pthread_self() << " task = "<< &task <<" start working..." << std::endl;
        task.function(task.arg);
        /**
         * 注意如果是堆内存可以在此处删除，是栈内存无需删除
         */
//        delete task.arg;
//        task.arg = nullptr;

        std::cout << "thread " << pthread_self() << " end working..." << std::endl;
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }

    return nullptr;
}


// 管理者线程任务函数
void *ThreadPool::manager(void *arg) {
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    // 如果线程池没有关闭, 就一直检测
    while (!pool->m_shutdown) {
        // 每隔5s检测一次
        sleep(5);
        // 取出线程池中的任务数和线程数量
        //  取出工作的线程池数量
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);
        std::cout << "管理线程开始工作：任务数 " << queueSize << " 活着的线程数 "<< liveNum << " busy " << busyNum << std::endl;
        for (int i = 0; i < liveNum; i ++) {
            if (pool->m_threadIDs[i] != 0) {
                std::cout<<"活着的线程 id = "<< pool->m_threadIDs[i] << std::endl;
            }
        }
        // 创建线程
        const int NUMBER = 2;
        // 当前任务个数>存活的线程数 && 存活的线程数<最大线程个数
        if (queueSize > liveNum && liveNum < pool->m_maxNum) {
            // 线程池加锁
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            std::cout << "任务数大于活着的线程数据，表现线程过小，人力资源不够，需要增派人手 管理将创建线程"<<std::endl;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER
                            && pool->m_aliveNum < pool->m_maxNum; ++i) {
                if (pool->m_threadIDs[i] == 0) {
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num++;
                    pool->m_aliveNum++;
                    std::cout << "管理线程临时 创建子线程, ID: " << pool->m_threadIDs[i] << std::endl;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // 销毁多余的线程
        // 忙线程*2 < 存活的线程数目 && 存活的线程数 > 最小线程数量
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum) {
            std::cout << "有空闲线程 需要销毁" << std::endl;
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            for (int i = 0; i < NUMBER; ++i) {
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    std::cout<<"管理者线程销毁"<<std::endl;
    return nullptr;
}

void ThreadPool::threadExit() {
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; ++i) {
        if (m_threadIDs[i] == tid) {
            std::cout << "threadExit() function: thread "<< pthread_self() << " exiting..." << std::endl;
            m_threadIDs[i] = 0;
            break;
        }
    }
    pthread_exit(nullptr);
}

void ThreadPool::shutDown() {
    m_shutdown = 1;
    for (int i = 0; i < m_aliveNum; ++i) {
        pthread_cond_signal(&m_notEmpty);
    }
    pthread_join(m_managerID, nullptr);
    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[]m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
    std::cout << "ThreadPool 线程池销毁" << std::endl;
}

