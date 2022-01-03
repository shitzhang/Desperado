#include "stdafx.h"
#include "Threading.h"

namespace Desperado
{
    namespace
    {
        struct ThreadingData
        {
            bool initialized = false;
            std::vector<std::thread> threads;
            uint32_t current;
        } gData;
    }

    void Threading::start(uint32_t threadCount)
    {
        if (gData.initialized) return;

        gData.threads.resize(threadCount);
        gData.initialized = true;
    }

    void Threading::shutdown()
    {
        for (auto& t : gData.threads)
        {
            if (t.joinable()) t.join();
        }

        gData.initialized = false;
    }

    Threading::Task Threading::dispatchTask(const std::function<void(void)>& func)
    {
        assert(gData.initialized);

        std::thread& t = gData.threads[gData.current];
        if (t.joinable()) t.join();
        t = std::thread(func);
        gData.current = (gData.current + 1) % gData.threads.size();

        return Task();
    }

    void Threading::finish()
    {
        for (auto& t : gData.threads)
        {
            if (t.joinable()) t.join();
        }
    }

    Threading::Task::Task()
    {
    }

    bool Threading::Task::isRunning()
    {
        printf("Threading::Task::isRunning() not implemented");
        return true;
    }

    void Threading::Task::finish()
    {
        printf("Threading::Task::finish() not implemented");
    }
}
