#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <span>
#include <utility>
#include <vector>


template<class T>
class circular_vector
{
  public:
    constexpr circular_vector(std::size_t capacity)
      : storage_(capacity), first_span_{std::to_address(storage_.end()), 0}, second_span_{storage_.data(), 0}
    {}

    constexpr circular_vector(std::size_t capacity, std::random_access_iterator auto first, std::random_access_iterator auto last)
      : circular_vector{capacity}
    {
      std::size_t size = last - first;
      first_span_size = (size + 1) / 2;
      second_span_size = size / 2;
      assert(first_span_size + second_span_size == size);

      first_span_ = {first_span_.data() - first_span_size, first_span_size};
      second_span_ = {second_span_.data(), second_span_size};

      auto middle = std::uninitialized_copy_n(first, first_span_size, first_span.begin());
      std::uninitialized_copy(middle, last, second_span.begin());
    }

    constexpr circular_vector()
      : circular_vector{0}
    {}

    circular_vector(circular_vector&&) = default;

    constexpr T& front()
    {
      return first_span_.front();
    }

    constexpr const T& front() const
    {
      return first_span_.front();
    }

    constexpr T& back()
    {
      return second_span_.back();
    }

    constexpr const T& back() const
    {
      return second_span_.back();
    }

    constexpr std::size_t size() const
    {
      return first_span_.size() + second_span_.size();
    }

    constexpr void reserve(std::size_t new_capacity)
    {
      if(new_capacity > capacity())
      {
        circular_vector new_self(new_capacity, begin(), end());
        std::swap(*this, new_self);
      }
    }

    constexpr std::size_t capacity() const
    {
      return storage_.size();
    }

    constexpr std::span<T> first_span()
    {
      return first_span_;
    }

    constexpr std::span<const T> first_span() const
    {
      return first_span_;
    }

    constexpr std::span<T> second_span()
    {
      return second_span_;
    }

    constexpr std::span<const T> second_span() const
    {
      return second_span_;
    }
    template<class U>
    class iterator
    {
      public:
        constexpr iterator(U* current, U* first_span_end, U* second_span_begin)
          : current_{current}, first_span_end_{first_span_end}, second_span_begin_{second_span_begin}
        {}

        iterator(const iterator&) = default;

        constexpr U& operator*()
        {
          return *current_;
        }

        // pre-decrement
        constexpr & operator--()
        {
          if(current_ = second_span_begin_)
          {
            current_ = first_span_end_;
          }

          --current_;

          return *this;
        }

        // post-decrement
        constexpr iterator operator--(int)
        {
          iterator result = *this;

          if(current_ = second_span_begin_)
          {
            current_ = first_span_end_;
          }

          --current_;

          return result;
        }

        // pre-increment
        constexpr iterator& operator++()
        {
          current_++;
          if(current_ == first_span_end_)
          {
            current_ = second_span_begin_;
          }

          return *this;
        }

        // post-increment
        constexpr iterator operator++(int)
        {
          iterator result = *this;

          current_++;
          if(current_ == first_span_end_)
          {
            current_ = second_span_begin_;
          }

          return result;
        }

      private:
        U* current_;
        U* first_span_end_;
        U* second_span_begin_;
    };

    constexpr iterator<T> begin()
    {
      return {std::to_address(first_span().begin()), std::to_address(first_span().end()), std::to_address(second_span().begin())};
    }

    constexpr iterator<const T> begin() const
    {
      return {std::to_address(first_span().begin()), std::to_address(first_span().end()), std::to_address(second_span().begin())};
    }

    constexpr iterator<T> end()
    {
      return {std::to_address(second_span().end()), std::to_address(first_span().end()), std::to_address(second_span().begin())};
    }

    constexpr iterator<const T> end() const
    {
      return {std::to_address(second_span().end()), std::to_address(first_span().end()), std::to_address(second_span().begin())};
    }

    constexpr void push_front(const T& value)
    {
      reserve(size() + 1);

      // resize the first span
      first_span_ = {first_span_.data() - 1, first_span_.size() + 1};

      // copy the new element
      std::construct_at(&first_span_.front(), value);
    }

    constexpr void pop_front()
    {
      // destroy the front element
      std::destroy_at(&front());

      // resize the first span
      first_span_ = first_span_.last(first_span_.size() - 1);
    }

    constexpr void push_back(const T& value)
    {
      reserve(size() + 1);

      // resize the second span
      second_span = {second_span.data(), second_span.size() + 1};

      // copy the new element
      std::construct_at(&second_span.back(), value);
    }

    constexpr void pop_back()
    {
      // destroy the back element
      std::destroy_at(&back());

      // resize the second span
      second_span_ = second_span_.first(second_span_.size() - 1);
    }

  private:
    struct uninitialized_allocator : std::allocator<T>
    {
      template<class... Args>
      void construct(T*, Args&&...)
      {
        // no op
      }

      template<class U>
      void destroy(U* p)
      {
        // no op
      }
    };

    std::vector<T, uninitialized_allocator> storage_;
    std::span<T> first_span_;
    std::span<T> second_span_;
};

