//
// Created by loobson on 2/25/22.
//

#ifndef PP01_PPQUEUE_H
#define PP01_PPQUEUE_H
#pragma once

#include "ppCommon.h"
#include <mutex>
#include <condition_variable>
#include "queue"
#include "chrono"


//===============================================================================================================================================================================================================
template<class XXX>
class ppQueue {
protected:
    std::queue<XXX> *m_Queue; // our Q
    int32_t m_QueueSize; // Q size

    std::mutex m_Mutex;
    std::condition_variable m_ConditionVariable;

public:
    ppQueue();

    void create(const int32_t QueueSize);

    void destroy();

    void EnqueueWait(const XXX &NodeData);

    XXX DequeueWait();

    bool isEmpty();

    bool isFull();

    int32_t getLoad();
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class XXX>
ppQueue<XXX>::ppQueue() {
    m_Queue = nullptr;
}

template<class XXX>
void ppQueue<XXX>::create(const int32_t QueueSize) {
    m_Queue = new std::queue<XXX>;
    m_QueueSize = QueueSize;
}

template<class XXX>
void ppQueue<XXX>::destroy() {
    free(m_Queue);
    m_Queue = nullptr;
}

template<class XXX>
void ppQueue<XXX>::EnqueueWait(const XXX &NodeData) {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while ((int32_t) m_Queue->size() >= m_QueueSize) {
        m_ConditionVariable.wait(LockManager, [&] {
            return (int32_t) m_Queue->size() < m_QueueSize;
        });
    }
    m_Queue->push(NodeData);
    m_ConditionVariable.notify_all();
    //release lock - std::unique_lock destructor...
}

template<class XXX>
XXX ppQueue<XXX>::DequeueWait() {
    XXX NodeData;
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while (m_Queue->empty()) { m_ConditionVariable.wait(LockManager, [&] { return not m_Queue->empty(); }); }
    NodeData = m_Queue->front();
    m_Queue->pop(); //doesnt return any value, so have to access the first element first (cuz FIFO)
    m_ConditionVariable.notify_all();
    //release lock - std::unique_lock destructor...
    return NodeData;
}

template<class XXX>
bool ppQueue<XXX>::isEmpty() {
    bool isEmpty;
    m_Mutex.lock();
    isEmpty = m_Queue->empty();
    m_Mutex.unlock();
    return isEmpty;
}

template<class XXX>
bool ppQueue<XXX>::isFull() {
    bool isFull;
    m_Mutex.lock();
    isFull = m_Queue->size() == m_QueueSize;
    m_Mutex.unlock();
    return isFull;
}

template<class XXX>
int32_t ppQueue<XXX>::getLoad() {
    int32_t NumElements;
    m_Mutex.lock();
    NumElements = m_Queue->size();
    m_Mutex.unlock();
    return NumElements;
}

//===============================================================================================================================================================================================================

#endif //PP01_PPQUEUE_H
