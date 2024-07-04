#pragma once
#include "config.hpp"
#ifndef BUILD_CRA3Z_UTIL_MODULE

#include <list>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <functional>
#include "move_only_function.hpp"

#endif

namespace cra3zutil {

    CRA3Z_MOD_EXPORT class thread_pool {

        using task_wrapper = move_only_function<void()>;

    public:

        explicit thread_pool(std::size_t worker_count_ = std::thread::hardware_concurrency()) : worker_cnt_(worker_count_) {
            for (std::size_t i{}; i < worker_cnt_; ++i) {
                workers_.emplace_back([this] {
                    this->fetch_and_finish_task_(this->stop_.get_token());
                });
                thread_ids_.push_back(workers_.back().get_id());
            }
        }

        thread_pool(const thread_pool&) = delete;

        thread_pool(thread_pool&&) = delete;

        ~thread_pool() {
            if (!stop_.stop_requested()) shutdown();
        }

        auto operator= (const thread_pool&) ->thread_pool& = delete;

        auto operator= (thread_pool&&) ->thread_pool& = delete;

        template<typename F, typename... Args> requires std::invocable<F, Args...>
        [[nodiscard]]
    	auto submit(F&& f, Args&&... args) ->std::future<std::invoke_result_t<F, Args...>> {
            if (stop_.stop_requested()) return std::async(std::launch::deferred, std::forward<F>(f), std::forward<Args>(args)...);
            auto task_args_ = std::tuple{std::forward<Args>(args)...};

            std::packaged_task<std::invoke_result_t<F, Args...>()> ptask{[f = std::forward<F>(f), task_args_ = std::move(task_args_)]() mutable ->decltype(auto) {
                return std::apply(std::forward<F>(f), task_args_);
            }};
            auto future_ = ptask.get_future();
            std::scoped_lock<std::mutex> lck{mtx_};
            task_queue_.emplace(std::move(ptask));
            cv_.notify_one();
            return future_;
        }

        auto join() ->void {
            for (auto&& worker : workers_) {
                if (worker.joinable()) worker.join();
            }
        }

        auto shutdown() ->void {
            if (!stop_.stop_requested()) {
                stop_.request_stop();
                cv_.notify_all();
                join();
            }
        }

    	[[nodiscard]]
        auto worker_count() noexcept ->std::size_t {
            return worker_cnt_;
        }

    	[[nodiscard]]
        auto worker_ids() noexcept ->const std::vector<std::thread::id>& {
            return thread_ids_;
        }
    
    private:

        auto fetch_and_finish_task_(std::stop_token token) ->void {
            for (;;) {
                std::unique_lock<std::mutex> ulk{mtx_};
                
                cv_.wait(ulk, [this, &token]{return !task_queue_.empty() || token.stop_requested();});
                
                if (task_queue_.empty()) return; // return directly if stop requested and the queue empty
                
                auto task = std::move(task_queue_.front());
                task_queue_.pop();
                ulk.unlock();
                
                task();
            }
        }
    
    private:
        std::size_t worker_cnt_;
        std::stop_source stop_;
        std::condition_variable cv_; // cv_: queue empty or pool stop
        std::mutex mtx_;
        std::list<std::thread> workers_;
        std::queue<task_wrapper> task_queue_;
        std::vector<std::thread::id> thread_ids_;
    };

}