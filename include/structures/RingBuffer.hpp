//
// Created by 280325 on 8/11/2025.
//

#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

template <typename T, size_t UPDATES>
struct RingBuffer {
private:
    T buffer[UPDATES];
    int head = 0, tail = 0;
public:
    inline void write(T entry) {
        buffer[head] = entry;
        head = (head + 1) % UPDATES;
    }

    inline T& read() {
        T& entry = buffer[tail];
        tail = (tail + 1) % UPDATES;
        return entry;
    }

    inline bool empty() const {
        return head == tail;
    }
};

#endif //RINGBUFFER_HPP
