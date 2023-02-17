// =============================================================================
// circular_buffer.h -- Simple circular buffer
// =============================================================================
#pragma once

#include <array>

namespace marguerite
{
template <size_t CONTAINER_SIZE> class Buffer
{
  public:
    Buffer() = default;
    ~Buffer() = default;

    void Reset()
    {
        Fill(0.f);
    }

    bool IsEmpty() const
    {
        return count_ == 0;
    }

    bool IsFull() const
    {
        return count_ == CONTAINER_SIZE;
    }

    size_t Count() const
    {
        return count_;
    }

    size_t Size() const
    {
        return CONTAINER_SIZE;
    }

    void Fill(float value)
    {
        buffer_.fill(0.f);
        count_ = buffer_.size();
    }

    float Read()
    {
        float val = buffer_[read_];
        read_ = (read_ + 1) % CONTAINER_SIZE;
        --count_;

        return val;
    }

    float Peek() const
    {
        return buffer_[read_];
    }

    void Write(float in)
    {
        buffer_[write_] = in;
        write_ = (write_ + 1) % CONTAINER_SIZE;
        ++count_;

        // Check if we overwrote the oldest data
        if (count_ > CONTAINER_SIZE)
        {
            read_ = (read_ + 1) % CONTAINER_SIZE;
            count_ = CONTAINER_SIZE;
        }
    }

    const float &operator[](size_t idx) const
    {
        auto real_idx = (read_ + idx) % CONTAINER_SIZE;
        return buffer_[real_idx];
    }

  private:
    std::array<float, CONTAINER_SIZE> buffer_ = {0};
    size_t count_ = 0;
    size_t write_ = 0;
    size_t read_ = 0;
};
} // namespace marguerite