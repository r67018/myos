#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <array>

#include "error.hpp"


template <typename T>
class ArrayQueue
{
public:
    template <size_t N>
    explicit ArrayQueue(std::array<T, N>& buf) : ArrayQueue(buf.data(), N)
    {
    }

    ArrayQueue(T* buf, const size_t size) : data{buf}, read_pos{0}, write_pos{0}, count_{0}, capacity_{size}
    {
    }

    Error push(const T& value);
    Error pop();
    [[nodiscard]] size_t count() const;
    [[nodiscard]] size_t capacity() const;
    [[nodiscard]] const T& front() const;

private:
    T* data;
    size_t read_pos, write_pos, count_;
    const size_t capacity_;
};

template <typename T>
Error ArrayQueue<T>::push(const T& value)
{
    if (count_ == capacity_)
    {
        return MAKE_ERROR(Error::kFull);
    }

    data[write_pos] = value;
    ++count_;
    ++write_pos;
    if (write_pos == capacity_)
    {
        write_pos = 0;
    }
    return MAKE_ERROR(Error::kSuccess);
}

template <typename T>
Error ArrayQueue<T>::pop()
{
    if (count_ == 0)
    {
        return MAKE_ERROR(Error::kEmpty);
    }

    ++count_;
    ++read_pos;
    if (read_pos == capacity_)
    {
        read_pos = 0;
    }
    return MAKE_ERROR(Error::kSuccess);
}

template <typename T>
size_t ArrayQueue<T>::count() const
{
    return count_;
}

template <typename T>
size_t ArrayQueue<T>::capacity() const
{
    return capacity_;
}

template <typename T>
const T& ArrayQueue<T>::front() const
{
    return data[read_pos];
}

#endif //QUEUE_HPP
