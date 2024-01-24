#pragma once
/*
  Original implementation:
  https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiJobSystem.cpp
  Cleaned up and segmented from the library.
  https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/
*/

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#ifdef PLATFORM_LINUX
#include <pthread.h>
#endif // PLATFORM_LINUX

namespace arc {
namespace core {
namespace JobManager {

const std::string impl_name = "arc::core";
const std::string threadname_prefix = impl_name + "::JobManager::";
const std::string job_prefix = impl_name + "::Job::";

std::atomic<bool> _is_initialized_{false};

bool ready() { return _is_initialized_; }

struct JobArgs {
    uint32_t job_index; // job index relative to dispatch (like
                        // SV_DispatchThreadID in HLSL)
    uint32_t
        group_ID; // group index relative to dispatch (like SV_GroupID in HLSL)
    uint32_t
        group_index; // job index relative to group (like SV_GroupIndex in HLSL)
    bool
        is_first_job_in_group; // is the current job the first one in the group?
    bool is_last_job_in_group; // is the current job the last one in the group?
    void* sharedmemory; // stack memory shared within the current group (jobs
                        // within a group execute serially)
};

// Defines a state of execution, can be waited on
struct Context {
    std::atomic<uint32_t> counter{0};
};

struct Timer {
    std::chrono::high_resolution_clock::time_point timestamp =
        std::chrono::high_resolution_clock::now();

    // Record a reference timestamp
    inline void record() {
        timestamp = std::chrono::high_resolution_clock::now();
    }

    // Elapsed time in seconds between the wi::Timer creation or last recording
    // and "timestamp2"
    inline double elapsed_seconds_since(
        std::chrono::high_resolution_clock::time_point timestamp2) {
        std::chrono::duration<double> time_span =
            std::chrono::duration_cast<std::chrono::duration<double>>(
                timestamp2 - timestamp);
        return time_span.count();
    }

    // Elapsed time in seconds since the wi::Timer creation or last recording
    inline double elapsed_seconds() {
        return elapsed_seconds_since(std::chrono::high_resolution_clock::now());
    }

    // Elapsed time in milliseconds since the wi::Timer creation or last
    // recording
    inline double elapsed_milliseconds() { return elapsed_seconds() * 1000.0; }

    // Elapsed time in milliseconds since the wi::Timer creation or last
    // recording
    inline double elapsed() { return elapsed_milliseconds(); }

    // Record a reference timestamp and return elapsed time in seconds since the
    // wi::Timer creation or last recording
    inline double record_elapsed_seconds() {
        auto timestamp2 = std::chrono::high_resolution_clock::now();
        auto elapsed = elapsed_seconds_since(timestamp2);
        timestamp = timestamp2;
        return elapsed;
    }
};

class SpinLock {
  private:
    std::atomic_flag lck = ATOMIC_FLAG_INIT;

  public:
    inline void lock() {
        int spin = 0;
        while (!try_lock()) {
            if (spin < 10) {
                //_mm_pause(); // SMT thread swap can occur here
            } else {
                std::this_thread::yield(); // OS thread swap can occur here. It
                                           // is important to keep it as
                                           // fallback, to avoid any chance of
                                           // lockup by busy wait
            }
            spin++;
        }
    }
    inline bool try_lock() {
        return !lck.test_and_set(std::memory_order_acquire);
    }

    inline void unlock() { lck.clear(std::memory_order_release); }
};

struct Job {
    std::function<void(JobArgs)> task;
    Context* context;
    uint32_t group_ID;
    uint32_t group_job_offset;
    uint32_t group_job_end;
    uint32_t sharedmemory_size;
};

struct JobQueue {
    std::deque<Job> queue;
    std::mutex locker;

    inline void push_back(const Job& item) {
        std::scoped_lock lock(locker);
        queue.push_back(item);
    }

    inline bool pop_front(Job& item) {
        std::scoped_lock lock(locker);
        if (queue.empty()) {
            return false;
        }
        item = std::move(queue.front());
        queue.pop_front();
        return true;
    }
};

// This structure is responsible to stop worker thread loops.
//	Once this is destroyed, worker threads will be woken up and end their loops.
struct InternalState {
    uint32_t n_cores = 0;
    uint32_t n_threads = 0;
    std::unique_ptr<JobQueue[]> job_queue_per_thread;
    std::atomic_bool alive{true};
    std::condition_variable wake_condition;
    std::mutex wake_mutex;
    std::atomic<uint32_t> next_queue{0};
    std::vector<std::thread> threads;
    void shutdown() {
        alive.store(
            false); // indicate that new jobs cannot be started from this point
        bool wake_loop = true;
        std::thread waker([&] {
            while (wake_loop) {
                wake_condition.notify_all(); // wakes up sleeping worker threads
            }
        });
        for (auto& thread : threads) {
            thread.join();
        }
        wake_loop = false;
        waker.join();
        job_queue_per_thread.reset();
        threads.clear();
        n_cores = 0;
        n_threads = 0;
    }
    ~InternalState() { shutdown(); }
} static internal_state;

// Start working on a job queue
//	After the job queue is finished, it can switch to an other queue and steal
// jobs from there
inline void work(uint32_t startingQueue) {
    Job job;
    for (uint32_t i = 0; i < internal_state.n_threads; ++i) {
        JobQueue& job_queue =
            internal_state
                .job_queue_per_thread[startingQueue % internal_state.n_threads];
        while (job_queue.pop_front(job)) {
            JobArgs args;
            args.group_ID = job.group_ID;
            if (job.sharedmemory_size > 0) {
                thread_local static std::vector<uint8_t> shared_allocation_data;
                shared_allocation_data.reserve(job.sharedmemory_size);
                args.sharedmemory = shared_allocation_data.data();
            } else {
                args.sharedmemory = nullptr;
            }

            for (uint32_t j = job.group_job_offset; j < job.group_job_end;
                 ++j) {
                args.job_index = j;
                args.group_index = j - job.group_job_offset;
                args.is_first_job_in_group = (j == job.group_job_offset);
                args.is_last_job_in_group = (j == job.group_job_end - 1);
                job.task(args);
            }

            job.context->counter.fetch_sub(1);
        }
        startingQueue++; // go to next queue
    }
}

void initialize(uint32_t maxThreadCount = 4) {
    if (internal_state.n_threads > 0)
        return;
    maxThreadCount = std::max(1u, maxThreadCount);

    Timer timer;

    // Retrieve the number of hardware threads in this system:
    internal_state.n_cores = std::thread::hardware_concurrency();

    // Calculate the actual number of worker threads we want (-1 main thread):
    internal_state.n_threads =
        std::min(maxThreadCount, std::max(1u, internal_state.n_cores - 1));
    internal_state.job_queue_per_thread.reset(
        new JobQueue[internal_state.n_threads]);
    internal_state.threads.reserve(internal_state.n_threads);

    for (uint32_t threadID = 0; threadID < internal_state.n_threads;
         ++threadID) {
        internal_state.threads.emplace_back([threadID] {
            while (internal_state.alive.load()) {
                work(threadID);

                // finished with jobs, put to sleep
                std::unique_lock<std::mutex> lock(internal_state.wake_mutex);
                internal_state.wake_condition.wait(lock);
            }
        });
        // std::thread& worker = internal_state.threads.back();

#ifdef _WIN32
        // Do Windows-specific thread setup:
        HANDLE handle = (HANDLE)worker.native_handle();

        // Put each thread on to dedicated core:
        DWORD_PTR affinityMask = 1ull << threadID;
        DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
        assert(affinity_result > 0);

        //// Increase thread priority:
        // BOOL priority_result = SetThreadPriority(handle,
        // THREAD_PRIORITY_HIGHEST); assert(priority_result != 0);

        // Name the thread:
        std::string wthreadname =
            L"ArcSystems::JobManager::" + std::to_string(threadID);
        HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
        assert(SUCCEEDED(hr));
#elif defined(PLATFORM_LINUX)
#define handle_error_en(en, msg)                                               \
    do {                                                                       \
        errno = en;                                                            \
        perror(msg);                                                           \
    } while (0)

        int ret;
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        size_t cpusetsize = sizeof(cpuset);

        CPU_SET(threadID, &cpuset);
        ret =
            pthread_setaffinity_np(worker.native_handle(), cpusetsize, &cpuset);
        if (ret != 0)
            handle_error_en(ret, std::string(" pthread_setaffinity_np[" +
                                             std::to_string(threadID) + ']')
                                     .c_str());

        // Name the thread
        std::string thread_name = "wi::job::" + std::to_string(threadID);
        ret = pthread_setname_np(worker.native_handle(), thread_name.c_str());
        if (ret != 0)
            handle_error_en(ret, std::string(" pthread_setname_np[" +
                                             std::to_string(threadID) + ']')
                                     .c_str());
#undef handle_error_en
#endif // _WIN32
    }

    // wi::backlog::post("wi::jobsystem Initialized with [" +
    // std::to_string(internal_state.numCores) + " cores] [" +
    // std::to_string(internal_state.numThreads) + " threads] (" +
    // std::to_string((int)std::round(timer.elapsed())) + " ms)");
    _is_initialized_ = true;
}

void shutdown() { internal_state.shutdown(); }

uint32_t get_thread_count() { return internal_state.n_threads; }

// Add a task to execute asynchronously. Any idle thread will execute this.
void execute(Context& ctx, const std::function<void(JobArgs)>& task) {
    // Context state is updated:
    ctx.counter.fetch_add(1);

    Job job;
    job.context = &ctx;
    job.task = task;
    job.group_ID = 0;
    job.group_job_offset = 0;
    job.group_job_end = 1;
    job.sharedmemory_size = 0;

    internal_state
        .job_queue_per_thread[internal_state.next_queue.fetch_add(1) %
                              internal_state.n_threads]
        .push_back(job);
    internal_state.wake_condition.notify_one();
}

uint32_t dispatch_group_count(uint32_t jobCount, uint32_t groupSize);
// Divide a task onto multiple jobs and execute in parallel.
//	jobCount	: how many jobs to generate for this task.
//	groupSize	: how many jobs to execute per thread. Jobs inside a group
// execute serially. It might be worth to increase for small jobs 	task :
// receives a JobArgs as parameter
void dispatch(Context& ctx, uint32_t jobCount, uint32_t groupSize,
              const std::function<void(JobArgs)>& task,
              size_t sharedmemory_size) {
    if (jobCount == 0 || groupSize == 0) {
        return;
    }

    const uint32_t groupCount = dispatch_group_count(jobCount, groupSize);

    // Context state is updated:
    ctx.counter.fetch_add(groupCount);

    Job job;
    job.context = &ctx;
    job.task = task;
    job.sharedmemory_size = (uint32_t)sharedmemory_size;

    for (uint32_t groupID = 0; groupID < groupCount; ++groupID) {
        // For each group, generate one real job:
        job.group_ID = groupID;
        job.group_job_offset = groupID * groupSize;
        job.group_job_end =
            std::min(job.group_job_offset + groupSize, jobCount);

        internal_state
            .job_queue_per_thread[internal_state.next_queue.fetch_add(1) %
                                  internal_state.n_threads]
            .push_back(job);
    }

    internal_state.wake_condition.notify_all();
}

// Returns the amount of job groups that will be created for a set number of
// jobs and group size
uint32_t dispatch_group_count(uint32_t jobCount, uint32_t groupSize) {
    // Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
    return (jobCount + groupSize - 1) / groupSize;
}

// Check if any threads are working currently or not
bool is_busy(const Context& ctx) {
    // Whenever the context label is greater than zero, it means that there is
    // still work that needs to be done
    return ctx.counter.load() > 0;
}

// Wait until all threads become idle
//	Current thread will become a worker thread, executing jobs
void wait_for(const Context& ctx) {
    if (is_busy(ctx)) {
        // Wake any threads that might be sleeping:
        internal_state.wake_condition.notify_all();

        // work() will pick up any jobs that are on stand by and execute them on
        // this thread:
        work(internal_state.next_queue.fetch_add(1) % internal_state.n_threads);

        while (is_busy(ctx)) {
            // If we are here, then there are still remaining jobs that work()
            // couldn't pick up.
            //	In this case those jobs are not standing by on a queue but
            // currently executing 	on other threads, so they cannot be picked
            // up by this thread. 	Allow to swap out this thread by OS to not
            // spin endlessly for nothing
            std::this_thread::yield();
        }
    }
}

} /*ns*/
} /*ns*/
} /*ns*/
