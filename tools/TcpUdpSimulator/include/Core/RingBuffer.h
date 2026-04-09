#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace TcpUdpSimulator
{

class RingBuffer
{
public:
    explicit RingBuffer(std::size_t initialCapacity = 4096)
        : buffer_(initialCapacity, 0)
    {
    }

    std::size_t Size() const
    {
        return size_;
    }

    bool Empty() const
    {
        return size_ == 0;
    }

    void Clear()
    {
        head_ = 0;
        size_ = 0;
    }

    void Push(const std::uint8_t* data, std::size_t length)
    {
        EnsureCapacity(size_ + length);

        for (std::size_t i = 0; i < length; ++i)
        {
            buffer_[(head_ + size_ + i) % buffer_.size()] = data[i];
        }

        size_ += length;
    }

    bool Peek(std::uint8_t* outData, std::size_t length) const
    {
        if (length > size_)
        {
            return false;
        }

        for (std::size_t i = 0; i < length; ++i)
        {
            outData[i] = buffer_[(head_ + i) % buffer_.size()];
        }

        return true;
    }

    bool Pop(std::uint8_t* outData, std::size_t length)
    {
        if (!Peek(outData, length))
        {
            return false;
        }

        head_ = (head_ + length) % buffer_.size();
        size_ -= length;
        return true;
    }

    bool Discard(std::size_t length)
    {
        if (length > size_)
        {
            return false;
        }

        head_ = (head_ + length) % buffer_.size();
        size_ -= length;
        return true;
    }

private:
    void EnsureCapacity(std::size_t required)
    {
        if (required <= buffer_.size())
        {
            return;
        }

        std::size_t newCapacity = std::max(buffer_.size() * 2, required);
        std::vector<std::uint8_t> newBuffer(newCapacity, 0);

        for (std::size_t i = 0; i < size_; ++i)
        {
            newBuffer[i] = buffer_[(head_ + i) % buffer_.size()];
        }

        buffer_.swap(newBuffer);
        head_ = 0;
    }

    std::vector<std::uint8_t> buffer_;
    std::size_t head_ = 0;
    std::size_t size_ = 0;
};

} // namespace TcpUdpSimulator
