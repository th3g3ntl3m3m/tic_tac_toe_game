#include <iostream>
#include <chrono>
#include <thread>

#include "ThreadPoolsUnified.hpp"

using namespace std::chrono_literals;

int main() {
    //base

    if(true) 
    {
        std::cout << "BasicThreadPool" << std::endl;
        IThreadPool* pool = new BasicThreadPool(3);

        for(int i = 0; i < 5; ++i) {
            pool->enqueue([i] { 
                std::cout << "task" << std::endl;
                std::this_thread::sleep_for(100ms); });
        }

        std::this_thread::sleep_for(1s);
        delete pool;
    }

    //priority
    if(true)
    {
        std::cout << "PriorityThreadPool" << std::endl;
        IThreadPool* pool = new PriorityThreadPool(3);

        pool->enqueue([]{
            std::cout << "Task with priority 1" << std::endl;
        }, TaskOptions{1});
        pool->enqueue([]{
            std::cout << "Task with priority 10" << std::endl;
        }, TaskOptions{10});
        pool->enqueue([]{
            std::cout << "Task with priority 5" << std::endl;
        }, TaskOptions{5});

        std::this_thread::sleep_for(1s);
        delete pool;
    }

    //Sharded
    if(true)
    {
        std::cout << "ShardedThreadPool" << std::endl;
        IThreadPool* pool = new ShardedThreadPool(2, 2);

        pool->enqueue([]{
            std::cout << "Task for Shard 0" << std::endl;
        }, TaskOptions{0, 0});
        pool->enqueue([]{
            std::cout << "Task for Shard 1" << std::endl;
        }, TaskOptions{0, 1});

        std::this_thread::sleep_for(1s);
        delete pool;
    }

    if(true)
    {
        std::cout << "\n=== PriorityShardedThreadPool ===" << std::endl;
        IThreadPool* pool = new PriorityShardedThreadPool(2, 2);

        pool->enqueue([]{
            std::cout << "Shard 0: task with priority 1" << std::endl;
        }, TaskOptions{1, 0});
        pool->enqueue([]{
            std::cout << "Shard 0: task with priority 10" << std::endl;
        }, TaskOptions{10, 0});
        pool->enqueue([]{
            std::cout << "Shard 1: task with priority 5" << std::endl;
        }, TaskOptions{5, 1});
        pool->enqueue([]{
            std::cout << "Shard 1: task with priority 15" << std::endl;
        }, TaskOptions{15, 1});

        std::this_thread::sleep_for(1s);
        delete pool;
    }

    std::cout << "Done" << std::endl;
    return 0;
}