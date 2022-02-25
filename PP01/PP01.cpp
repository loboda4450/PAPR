#include "ppCommon.h"
#include <thread>
#include <future>
#include <chrono>
#include <functional>
#include <vector>
#include "ppRing.h"
#include "ppQueue.h"

#define FMT_HEADER_ONLY

#include "lib_fmt/format.h"
#include <random>
#include "lib_fmt/ostream.h"
#include "../PP02/CommonDef.h"

#undef FMT_HEADER_ONLY

//=============================================================================================================================================================================
// simple thread creation
//=============================================================================================================================================================================
int32_t ThreadFunctionSimple() {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START\n", ThreadID);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return EXIT_SUCCESS;
}

int32_t Example01() {
    fmt::print("MAIN Creating new thread\n");
    std::thread Thread = std::thread(ThreadFunctionSimple);
    fmt::print("MAIN Waiting...\n");
    Thread.join(); //it's blocking :(
    fmt::print("MAIN Done\n");
    return EXIT_SUCCESS;
}

int32_t Example02() {
    fmt::print("MAIN Creating new thread\n");
    std::packaged_task<int32_t()> PackagedTask(ThreadFunctionSimple); // wrap the function
    std::future<int32_t> Future = PackagedTask.get_future(); // get a future
    std::thread Thread = std::thread(std::move(PackagedTask)); // fire a thread
    while (1) {
        std::future_status Status = Future.wait_for(std::chrono::milliseconds(500));
        if (Status != std::future_status::ready) { fmt::print("MAIN pretend to do some work here...\n"); }
        else { break; }
    }
    int32_t Result = Future.get();
    Thread.join();
    fmt::print("MAIN RESULT={}\n", Result);
    fmt::print("MAIN Done\n");
    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
// pasing args & multiple threads
//=============================================================================================================================================================================
int32_t ThreadFunctionWithArgs(int32_t WaitMilliSeconds) {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START and wait {}ms\n", ThreadID, WaitMilliSeconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(WaitMilliSeconds));
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return EXIT_SUCCESS;
}

int32_t Example03() {
    fmt::print("MAIN Creating new thread\n");
    auto Wrapper = std::bind(ThreadFunctionWithArgs, 3000);
    std::packaged_task<int32_t()> PackagedTask(Wrapper);
    std::future<int32_t> Future = PackagedTask.get_future();
    std::thread Thread = std::thread(std::move(PackagedTask));
    while (1) {
        std::future_status Status = Future.wait_for(std::chrono::milliseconds(500));
        if (Status != std::future_status::ready) { fmt::print("MAIN Some work in main thread...\n"); }
        else { break; }
    }
    int32_t Result = Future.get();
    Thread.join();
    fmt::print("MAIN Result={}\n", Result);
    fmt::print("MAIN Done\n");
    return EXIT_SUCCESS;
}

int32_t ThreadFunctionWithArgsTwo(int32_t WaitMilliSeconds, int32_t RetVal) {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START and wait {}ms\n", ThreadID, WaitMilliSeconds);
    std::this_thread::sleep_for(std::chrono::milliseconds(WaitMilliSeconds));
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return RetVal;
}

int32_t Example04() {
    constexpr int32_t NumThreads = 100;
    std::vector<std::future<int32_t>> Futures;
    std::vector<std::thread> Threads;
    std::default_random_engine gen;
    std::uniform_real_distribution<float_t> delay(500, 3000);
    fmt::print("MAIN Creating many threads\n");
    for (int32_t ThreadCnt = 0; ThreadCnt < NumThreads; ThreadCnt++) {
        auto Wrapper = std::bind(ThreadFunctionWithArgsTwo, delay(gen), ThreadCnt);
        std::packaged_task<int32_t()> PackagedTask(Wrapper);
        Futures.push_back(PackagedTask.get_future());
        std::thread Thread = std::thread(std::move(PackagedTask));
        Threads.push_back(std::move(Thread));
    }

    fmt::print("MAIN Waiting for all threads...\n");
    while (1) {
        bool AllFinished = true;
        for (int32_t ThreadCnt = 0; ThreadCnt < NumThreads; ThreadCnt++) {
            std::future_status Status = Futures[ThreadCnt].wait_for(std::chrono::milliseconds(0));
            if (Status != std::future_status::ready) {
                AllFinished = false;
                break;
            }
        }
        if (AllFinished) {
            break;
        } else {
            fmt::print("MAIN still waiting...\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    fmt::print("MAIN all threads done...\n");
    for (int32_t ThreadCnt = 0; ThreadCnt < NumThreads; ThreadCnt++) {
        int32_t Result = Futures[ThreadCnt].get();
        Threads[ThreadCnt].join();
        fmt::print("MAIN Result[{}]={}\n", ThreadCnt, Result);
    }

    fmt::print("MAIN Done\n");
    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
// mutex
//=============================================================================================================================================================================
int32_t ThreadFunctionWithLock(int32_t *Resource, std::mutex *Mutex) {
    std::thread::id ThreadID = std::this_thread::get_id();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (Mutex != nullptr) {
        Mutex->lock();
        fmt::print("THRD ID={} AQUIRED LOCK\n", ThreadID);
    }

    *Resource += 1;

    if (Mutex != nullptr) {
        Mutex->unlock();
        fmt::print("THRD ID={} RELEASED LOCK\n", ThreadID);
    }

    return EXIT_SUCCESS;
}


int32_t Example05() {
#if defined(_MSC_VER)
    constexpr int32_t NumThreads = 100; //on Windows use 100 threads
#else
    constexpr int32_t NumThreads = 100; //on Linux/Unix use 10000 threads
#endif

    int32_t Resource = 10;
    std::mutex Mutex;

    std::vector<std::future<int32_t>> Futures;
    std::vector<std::thread> Threads;
    auto start = std::chrono::high_resolution_clock::now();
    fmt::print("MAIN Creating many threads\n");
    for (int32_t ThreadCnt = 0; ThreadCnt < NumThreads; ThreadCnt++) {
//    auto Wrapper = std::bind(ThreadFunctionWithLock, &Resource, nullptr);
        auto Wrapper = std::bind(ThreadFunctionWithLock, &Resource, &Mutex);
        std::packaged_task<int32_t()> PackagedTask(Wrapper);
        Futures.push_back(PackagedTask.get_future());
        std::thread Thread = std::thread(std::move(PackagedTask));
        Threads.push_back(std::move(Thread));
    }

    fmt::print("MAIN Waiting for all threads...\n");
    while (1) {
        bool AllFinished = true;
        for (int32_t ThreadCnt = 0; ThreadCnt < NumThreads; ThreadCnt++) {
            std::future_status Status = Futures[ThreadCnt].wait_for(std::chrono::milliseconds(0));
            if (Status != std::future_status::ready) {
                AllFinished = false;
                break;
            }
        }
        if (AllFinished) { break; }
        else { std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
    }

    fmt::print("MAIN all threads done...\n");
    for (int32_t ThreadCnt = 0; ThreadCnt < NumThreads; ThreadCnt++) {
        int32_t Result = Futures[ThreadCnt].get();
        if (Result != 0) { fmt::print("Result={} (at ThreadCnt {})\n", Result, ThreadCnt); }
        Threads[ThreadCnt].join();
    }
    fmt::print("THREADCNT={}\n", NumThreads);
    fmt::print("TIME={} ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start).count());
    fmt::print("RESOURCE={}\n", Resource);

    fmt::print("MAIN Done\n");
    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
// FIFO
//=============================================================================================================================================================================

constexpr static int32_t TestArraySize = 24;
tDuration TotalProducerDuration;
tDuration TotalConsumerDuration;
uint64_t TotalProducerNumTicks = 0;
uint64_t TotalConsumerNumTicks = 0;
struct ppTestObject {
    int32_t m_Idx;
    int32_t m_Array[TestArraySize];
};

int32_t ThreadFunctionProducer(ppRing<ppTestObject *> *Ring, int32_t NumTestObjects) {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START\n", ThreadID);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //production
    fmt::print("THRD ID={} production started\n", ThreadID);
    tTimePoint Start = tClock::now();
    uint64_t StartTick = __rdtsc();
    for (int32_t Idx = 0; Idx < NumTestObjects; Idx++) {
        ppTestObject *TestObject = new ppTestObject;
        TestObject->m_Idx = Idx;
        for (int32_t i = 0; i < TestArraySize; i++) { TestObject->m_Array[i] = Idx + i; }
        Ring->EnqueueWait(TestObject);
        fmt::print("THRD ID={} enqueued {}\n", ThreadID, Idx);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //signal end
    ppTestObject *LastObject = new ppTestObject;
    LastObject->m_Idx = -1;
    Ring->EnqueueWait(LastObject);
    TotalProducerDuration += tClock::now() - Start;
    TotalProducerNumTicks += __rdtsc() - StartTick;
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return EXIT_SUCCESS;
}

int32_t ThreadFunctionConsumer(ppRing<ppTestObject *> *Ring) {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START\n", ThreadID);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    fmt::print("THRD ID={} consumption started\n", ThreadID);
    tTimePoint Start = tClock::now();
    uint64_t StartTick = __rdtsc();
    int32_t Counter = 0;
    while (true) {
        ppTestObject *TestObject = Ring->DequeueWait();
        int32_t Idx = TestObject->m_Idx;
        delete TestObject;
        fmt::print("THRD ID={} dequeued {}\n", ThreadID, Idx);
        if (Idx == -1) { break; }
        if (Idx != Counter) { fmt::print("THRD ID={} something is wrong Idx={} Cnt={}\n", Idx, Counter); }
        Counter++;
    }

    fmt::print("THRD ID={} FINISH\n", ThreadID);
    TotalConsumerDuration += tClock::now() - Start;
    TotalConsumerNumTicks += __rdtsc() - StartTick;
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return Counter;
}

int32_t Example06() {
    ppRing<ppTestObject *> Ring;
    Ring.create(10);
    uint8_t TestObjectsCount = 100;

    fmt::print("MAIN Creating new thread\n");
    std::packaged_task<int32_t()> PackagedTaskProducer(std::bind(ThreadFunctionProducer, &Ring, TestObjectsCount));
    std::packaged_task<int32_t()> PackagedTaskConsumer(std::bind(ThreadFunctionConsumer, &Ring));
    std::future<int32_t> FutureProducer = PackagedTaskProducer.get_future();
    std::future<int32_t> FutureConsumer = PackagedTaskConsumer.get_future();
    std::thread ThreadProducer = std::thread(std::move(PackagedTaskProducer));
    std::thread ThreadConsumer = std::thread(std::move(PackagedTaskConsumer));
    while (1) {
        std::future_status Status = FutureConsumer.wait_for(std::chrono::milliseconds(500));
        if (Status == std::future_status::ready) { break; }
    }
    int32_t Result = FutureConsumer.get();
    ThreadProducer.join();
    ThreadConsumer.join();
    fmt::print("MAIN RESULT={}\n", Result);
    fmt::print("MAIN Done\n");
    fmt::print("AvgProdMs= {} ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(TotalProducerDuration).count() / TestObjectsCount);
    fmt::print("AvgConsMs= {} ms\n",std::chrono::duration_cast<std::chrono::milliseconds>(TotalConsumerDuration).count() / TestObjectsCount);
    fmt::print("AvgProdNumTicks= {}\n", TotalProducerNumTicks / TestObjectsCount);
    fmt::print("AvgConsNumTicks= {}\n", TotalConsumerNumTicks / TestObjectsCount);
    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
// FIFO USING STD::QUEUE
//=============================================================================================================================================================================

int32_t ThreadFunctionProducerQueue(ppQueue<ppTestObject *> *Queue, int32_t NumTestObjects) {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START\n", ThreadID);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //production
    fmt::print("THRD ID={} production started\n", ThreadID);
    tTimePoint Start = tClock::now();
    uint64_t StartTick = __rdtsc();
    for (int32_t Idx = 0; Idx < NumTestObjects; Idx++) {
        ppTestObject *TestObject = new ppTestObject;
        TestObject->m_Idx = Idx;
        for (int32_t i = 0; i < TestArraySize; i++) { TestObject->m_Array[i] = Idx + i; }
        Queue->EnqueueWait(TestObject);
        fmt::print("THRD ID={} enqueued {}\n", ThreadID, Idx);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //signal end
    ppTestObject *LastObject = new ppTestObject;
    LastObject->m_Idx = -1;
    Queue->EnqueueWait(LastObject);
    TotalProducerDuration += tClock::now() - Start;
    TotalProducerNumTicks += __rdtsc() - StartTick;
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return EXIT_SUCCESS;
}

int32_t ThreadFunctionConsumerQueue(ppQueue<ppTestObject *> *Queue) {
    std::thread::id ThreadID = std::this_thread::get_id();
    fmt::print("THRD ID={} START\n", ThreadID);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    fmt::print("THRD ID={} consumption started\n", ThreadID);
    tTimePoint Start = tClock::now();
    uint64_t StartTick = __rdtsc();
    int32_t Counter = 0;
    while (true) {
        ppTestObject *TestObject = Queue->DequeueWait();
        int32_t Idx = TestObject->m_Idx;
        delete TestObject;
        fmt::print("THRD ID={} dequeued {}\n", ThreadID, Idx);
        if (Idx == -1) { break; }
        if (Idx != Counter) { fmt::print("THRD ID={} something is wrong Idx={} Cnt={}\n", Idx, Counter); }
        Counter++;
    }

    fmt::print("THRD ID={} FINISH\n", ThreadID);
    TotalConsumerDuration += tClock::now() - Start;
    TotalConsumerNumTicks += __rdtsc() - StartTick;
    fmt::print("THRD ID={} FINISH\n", ThreadID);
    return Counter;
}

int32_t Example06Queue() {
    ppQueue<ppTestObject *> Queue;
    Queue.create(10);
    uint8_t TestObjectsCount = 100;
    fmt::print("MAIN Creating new thread\n");
    std::packaged_task<int32_t()> PackagedTaskProducer(std::bind(ThreadFunctionProducerQueue, &Queue, TestObjectsCount));
    std::packaged_task<int32_t()> PackagedTaskConsumer(std::bind(ThreadFunctionConsumerQueue, &Queue));
    std::future<int32_t> FutureProducer = PackagedTaskProducer.get_future();
    std::future<int32_t> FutureConsumer = PackagedTaskConsumer.get_future();
    std::thread ThreadProducer = std::thread(std::move(PackagedTaskProducer));
    std::thread ThreadConsumer = std::thread(std::move(PackagedTaskConsumer));
    while (true) {
        std::future_status Status = FutureConsumer.wait_for(std::chrono::milliseconds(500));
        if (Status == std::future_status::ready) { break; }
    }
    int32_t Result = FutureConsumer.get();
    ThreadProducer.join();
    ThreadConsumer.join();
    fmt::print("MAIN RESULT={}\n", Result);
    fmt::print("MAIN Done\n");
    fmt::print("AvgProdMs= {} ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(TotalProducerDuration).count() / TestObjectsCount);
    fmt::print("AvgConsMs= {} ms\n",std::chrono::duration_cast<std::chrono::milliseconds>(TotalConsumerDuration).count() / TestObjectsCount);
    fmt::print("AvgProdNumTicks= {}\n", TotalProducerNumTicks / TestObjectsCount);
    fmt::print("AvgConsNumTicks= {}\n", TotalConsumerNumTicks / TestObjectsCount);
    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
// time is money
//=============================================================================================================================================================================
using tClock = std::chrono::high_resolution_clock;
using tTimePoint = tClock::time_point;
using tDuration = tClock::duration;
using tDurationMS = std::chrono::duration<double, std::milli>;

int32_t Example07() {
    constexpr int32_t NumIter = 100;
    tDuration TotalDuration = tDuration(0);

    for (int32_t i = 0; i < NumIter; i++) {
        tTimePoint Beg = tClock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        tTimePoint End = tClock::now();
        tDuration Duration = End - Beg;
        TotalDuration += Duration;
    }

    double TotalMilliseconds = std::chrono::duration_cast<tDurationMS>(TotalDuration).count();
    double AverageMilliseconds = TotalMilliseconds / NumIter;

    fmt::print("TotalMilliseconds   = {}\n", TotalMilliseconds);
    fmt::print("AverageMilliseconds = {}\n", AverageMilliseconds);

    return EXIT_SUCCESS;
}


int32_t Example08() {
    constexpr int32_t NumIter = 100;
    uint64_t TotalNumTicks = 0;

    for (int32_t i = 0; i < NumIter; i++) {
        uint64_t Beg = __rdtsc();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        uint64_t End = __rdtsc();
        uint64_t NumTicks = End - Beg;
        TotalNumTicks += NumTicks;
    }

    double AverageNumTicks = (double) TotalNumTicks / (double) NumIter;

    fmt::print("TotalNumTicks   = {}\n", TotalNumTicks);
    fmt::print("AverageNumTicks = {}\n", AverageNumTicks);

    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
// MAIN
//=============================================================================================================================================================================
int main(int /*argc*/, char ** /*argv*/, char ** /*envp*/) {
    return Example06Queue();
}

