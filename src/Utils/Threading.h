
#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace Desperado
{
    class Threading
    {
    public:
        const static uint32_t kDefaultThreadCount = 16;

        /** Handle to a dispatched task

            TODO: Implementation
        */
        class Task
        {
        public:
            /** Check if task is still executing
            */
            bool isRunning();

            /** Wait for task to finish executing
            */
            void finish();

        private:
            Task();
            friend class Threading;
        };

        /** Initializes the global thread pool
            \param[in] threadCount Number of threads in the pool
        */
        static void start(uint32_t threadCount = kDefaultThreadCount);

        /** Waits for all currently executing threads to finish
        */
        static void finish();

        /** Waits for all currently executing threads to finish and shuts down the thread pool
        */
        static void shutdown();

        /** Returns the maximum number of concurrent threads supported by the hardware
        */
        static uint32_t getLogicalThreadCount() { return std::thread::hardware_concurrency(); }

        /** Starts a task on an available thread.
            \return Handle to the task
        */
        static Task dispatchTask(const std::function<void(void)>& func);
    };

    /** Simple thread barrier class.
        TODO: Once we move to C++20, we should change users of Barrier to use std::barrier instead.
        The only change necessary will be to use std::barrier::arrive_and_wait() in place of Barrier::wait().
    */
    class Barrier
    {
    public:
        Barrier(size_t threadCount, std::function<void()> completionFunc = nullptr)
            : mThreadCount(threadCount)
            , mWaitCount(threadCount)
            , mCompletionFunc(completionFunc)
        {}

        Barrier(const Barrier& barrier) = delete;
        Barrier& operator=(const Barrier& barrier) = delete;

        void wait()
        {
            std::unique_lock<std::mutex> lock(mMutex);

            auto generation = mGeneration;

            if (--mWaitCount == 0)
            {
                if (mCompletionFunc) mCompletionFunc();
                ++mGeneration;
                mWaitCount = mThreadCount;
                mCondition.notify_all();
            }
            else
            {
                mCondition.wait(lock, [this, generation] () { return generation != mGeneration; });
            }
        }

    private:
        size_t mThreadCount;
        size_t mWaitCount;
        size_t mGeneration = 0;
        std::function<void()> mCompletionFunc;
        std::mutex mMutex;
        std::condition_variable mCondition;
    };
}
