#pragma once

#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

using Task = std::function<void()>;

struct TaskOptions {
    int priority = 0;
    size_t shardId = 0;
    // to do more fields
};

class IThreadPool {
public:
    virtual void enqueue(Task task, const TaskOptions & options = {}) = 0;
    virtual ~IThreadPool() {}
    // todo more options
};

class BasicThreadPool : public IThreadPool {
public:
    explicit BasicThreadPool(size_t threadCount) : stopFlag(false) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this]{
                while (true) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this]{ return stopFlag || !tasks.empty(); });
                        if (stopFlag && tasks.empty())
                            return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~BasicThreadPool() override {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        condition.notify_all();
        for (auto &worker : workers)
            worker.join();
    }

    void enqueue(Task task, const TaskOptions & options = {}) override {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<Task> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stopFlag;
};

struct PrioritizedTask {
    int priority;
    Task task;
    bool operator<(const PrioritizedTask & other) const { return priority < other.priority; }
}; 

class PriorityThreadPool : public IThreadPool {
public:
    explicit PriorityThreadPool(size_t threadCount) : stopFlag(false) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this]{
                while (true) {
                    PrioritizedTask pTask;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this]{ return stopFlag || !priorityTasks.empty(); });
                        if (stopFlag && priorityTasks.empty())
                            return;
                        pTask = std::move(priorityTasks.top());
                        priorityTasks.pop();
                    }
                    pTask.task();
                }
            });
        }
    }

    ~PriorityThreadPool() override {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        condition.notify_all();
        for (auto &worker : workers)
            worker.join();
    }

    void enqueue(Task task, const TaskOptions & options = {}) override {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            priorityTasks.push(PrioritizedTask{options.priority, std::move(task)});
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::priority_queue<PrioritizedTask> priorityTasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stopFlag;
};

class ShardedThreadPool : public IThreadPool {
public:
    ShardedThreadPool(size_t shardCount, size_t threadsPerShard) {
        for (size_t i = 0; i < shardCount; ++i) {
            shards.push_back(new BasicThreadPool(threadsPerShard));
        }
    }

    ~ShardedThreadPool() override {
        for (auto shard : shards)
            delete shard;
    }

    void enqueue(Task task, const TaskOptions & options = {}) override {
        size_t id = options.shardId < shards.size() ? options.shardId : 0;
        shards[id]->enqueue(std::move(task));
    }

private:
    std::vector<IThreadPool*> shards;
};

class PriorityShardedThreadPool : public IThreadPool {
public:
    PriorityShardedThreadPool(size_t shardCount, size_t threadsPerShard) {
        for (size_t i = 0; i < shardCount; ++i) {
            shards.push_back(new PriorityThreadPool(threadsPerShard));
        }
    }

    ~PriorityShardedThreadPool() override {
        for (auto shard : shards)
            delete shard;
    }

    void enqueue(Task task, const TaskOptions & options = {}) override {
        size_t id = options.shardId < shards.size() ? options.shardId : 0;
        shards[id]->enqueue(std::move(task), options);
    }

private:
    std::vector<IThreadPool*> shards;
};
