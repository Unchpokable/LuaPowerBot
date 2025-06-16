#include "workers.hxx"

#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace workers::internal
{

namespace data
{

struct TaskInternal
{
    std::uint32_t uid {0};
    Task task;
    Callback on_success;
    Callback on_fail;

    TaskInternal& operator=(TaskInternal&& other) noexcept
    {
        task = std::move(other.task);
        on_success = std::move(other.on_success);
        on_fail = std::move(other.on_fail);

        return *this;
    }
};

std::queue<TaskInternal> tasks_queue;
std::mutex queue_mutex;
std::condition_variable queue_condition;
std::jthread worker_thread;

}

void worker_thread(const std::stop_token& stop_token)
{
    while(!stop_token.stop_requested()) {
        data::TaskInternal task;

        {
            std::unique_lock lock(data::queue_mutex);
            data::queue_condition.wait(lock, [] {
                return !data::tasks_queue.empty();
            });

            if(!data::tasks_queue.empty()) {
                task = std::move(data::tasks_queue.front());
                data::tasks_queue.pop();
            }
        }

        try {
            auto result = task.task();
            if(!result) {
                if(task.on_fail) {
                    task.on_fail(ExecResult { task.uid, Worker_TaskReturnedError, {}, result });
                }
            }

            if(task.on_success) {
                task.on_success(ExecResult { task.uid, Worker_Ok, {}, result });
            }

        } catch(const std::exception& ex) {
            if(task.on_fail) {
                task.on_fail(ExecResult { task.uid, Worker_TaskThrownException, ex, "" });
            }
        }
    }
}

}

namespace workers {

void initialize()
{
    internal::data::worker_thread = std::jthread(internal::worker_thread);
}

void shutdown()
{
    internal::data::worker_thread.request_stop();
    internal::data::worker_thread.join();
}

void execute(Task task, Callback on_success, Callback on_fail)
{
    {
        std::scoped_lock lock(internal::data::queue_mutex);
        internal::data::tasks_queue.push(internal::data::TaskInternal { 0, std::move(task), std::move(on_success), std::move(on_fail) });
    }

    internal::data::queue_condition.notify_all();
}

}

