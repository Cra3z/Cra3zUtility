#pragma once
#include "config.hpp"
#ifndef BUILD_CRA3Z_UTIL_MODULE

#include <concepts>
#include <coroutine>
#include <iterator>
#include <ranges>
#include <optional>
#include <stdexcept>

#endif

namespace cra3zutil {

	CRA3Z_MOD_EXPORT struct stop_iteration : std::runtime_error {
	    using std::runtime_error::runtime_error;
	};

	CRA3Z_MOD_EXPORT template<std::semiregular T>
	class generator {
	public:
	    struct promise_type;

	    using value_type = T;
	    using handle_type = std::coroutine_handle<promise_type>;

	    struct promise_type {
	        explicit promise_type() = default;
	        auto initial_suspend() noexcept ->std::suspend_always {
	            return {};
	        };
	        auto final_suspend() noexcept ->std::suspend_always {
	            return {};
	        };
	        auto return_void() ->void {
	            finished = true;
	        }
	        auto unhandled_exception() ->void {}
	        auto get_return_object() ->generator {
	            return {handle_type::from_promise(*this)};
	        }
	        auto yield_value(value_type value_) ->std::suspend_always {
	            value = std::move(value_);
	            return {};
	        }
	        value_type value;
	        bool has_begun_to_yield{false};
	        bool finished{false};
	    };

	    struct iterator {
	        using iterator_category = std::input_iterator_tag;
	        using value_type = T;
	        using difference_type = std::ptrdiff_t;
	        using pointer = value_type*;
	        using reference = value_type&;

	        auto operator++() noexcept ->iterator& {
	            return *this;
	        }
	        auto operator++(int) noexcept ->iterator& {
	            return *this;
	        }
	        auto operator*() const ->value_type {
	            return std::move(handle_.promise().value);
	        }
	        auto operator== (iterator) const ->bool {
	            if (!handle_) return true;
	            if (!handle_.done()) handle_.resume();
	            return handle_.promise().finished;
	        }
	        auto operator!= (iterator other) const ->bool {
	            return !(*this == other);
	        }

	        handle_type handle_;
	    };

	public:
	    generator() = default;
	    generator(handle_type handle_) noexcept : handle(handle_) {}
	    generator(const generator&) = delete;
	    generator(generator&& other) noexcept : handle(std::exchange(other.handle, {})) {}

	    auto operator= (const generator&) ->generator& = delete;
	    auto operator= (generator&& other) noexcept ->generator& {
	        handle = std::exchange(other.handle, {});
	    }

	    ~generator() noexcept {
	        if (handle) handle.destroy();
	    }

	    [[nodiscard]]
	    auto begin() noexcept ->iterator {
	        return {handle};
	    }

	    [[nodiscard]]
	    auto end() noexcept ->iterator {
	        return {handle};
	    }

	    [[nodiscard]]
	    auto next() ->std::optional<value_type> {
	        if (!resume_and_check_finished_()) return yield_one_();
	        return std::nullopt;
	    }

	    template<std::convertible_to<value_type> U>
	    [[nodiscard]]
	    auto next_or(U&& u) ->value_type {
	        if (!resume_and_check_finished_()) return yield_one_();
	        return std::forward<U>(u);
	    }

	    [[nodiscard]]
	    auto next_or_exception() ->value_type {
	        if (!resume_and_check_finished_()) return yield_one_();
	        throw stop_iteration{"this generator has been closed or finished"};
	    }

	    [[nodiscard]]
	    auto close() noexcept ->void {
	        handle = nullptr;
	    }

	    [[nodiscard]]
	    auto associated_with_coroutine(std::coroutine_handle<> coro_handle) noexcept ->bool {
	        if (coro_handle.address() == nullptr) return false;
	        return handle.address() == coro_handle.address();
	    }

	    [[nodiscard]]
	    auto has_associated_coroutine() noexcept ->bool {
	        return static_cast<bool>(handle);
	    }

	private:
	    auto resume_and_check_finished_() noexcept ->bool {
	        if (!handle) return true;
	        if (!handle.done()) handle.resume();
	        return handle.promise().finished;
	    }

	    auto yield_one_() ->value_type {
	        return std::move(handle.promise().value);
	    }
	public:
	    template<std::ranges::range Rng>
	    [[nodiscard]]
		static auto yield_from(Rng& rng) ->generator requires std::convertible_to<typename std::iterator_traits<decltype(std::ranges::begin(rng))>::value_type, value_type> {
	        for (auto&& i : rng) {
	            co_yield i;
	        }
	    }
	    template<std::forward_iterator ForwardIt> requires std::convertible_to<typename std::iterator_traits<ForwardIt>::value_type, value_type>
	    [[nodiscard]]
		static auto yield_from(ForwardIt first, ForwardIt last) ->generator {
	        for (auto it = first; it != last; ++it) {
	            co_yield *it;
	        }
	    }
	private:
	    handle_type handle;
	};

}