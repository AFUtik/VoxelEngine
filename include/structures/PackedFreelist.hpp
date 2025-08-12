//
// Created by 280325 on 8/11/2025.
//

#ifndef PACKEDFREELIST_HPP
#define PACKEDFREELIST_HPP

#include <cstdint>

template<typename T>
class packed_freelist {
    T* data;
    int* next;

    int32_t free_head;

    uint32_t capacity;
    uint32_t size;
public:
    packed_freelist(const size_t &n) : data(new T[n]), next(new int[n]), free_head(0), capacity(n), size(0) {
        for (int i = 0; i < n - 1; ++i) next[i] = i + 1;

        next[n - 1] = -1;
    }

    ~packed_freelist() {
        delete[] data;
        delete[] next;
    }



    void resize(const size_t &new_capacity) {
        if (new_capacity <= capacity) return;

        T* new_data = new T[new_capacity];
        int* new_next = new int[new_capacity];

        for (int i = 0; i < capacity; i++) {
            new_data[i] = data[i];
            new_next[i] = next[i];
        }

        for(int i = capacity; i < new_capacity - 1; i++) new_next[i] = i + 1;

        new_next[new_capacity - 1] = -1;

        delete[] data;
        delete[] next;

        data = new_data;
        next = new_next;
        free_head = capacity;
        capacity = new_capacity;
    }



    inline void push(const T& object) {
        if (size >= capacity) resize(capacity*2);

        data[size-1] = object;

        size++;
    }

    inline void allocate(const T& object) {
        if (free_head == -1) resize(capacity * 2);


        int index = free_head;
        free_head = next[free_head];
        data[index] = object;

        if (index >= size) size = index + 1;
    }

    inline void deallocate(int index) {
        next[index] = free_head;
        free_head = index;
    }

    inline T* get_data() {return data;}

    inline uint32_t get_size() {return size;}

    T& operator[](const size_t &index) {
        return data[index];
    }

    const T& operator[](const size_t &index) const {
        return data[index];
    }
};

#endif //PACKEDFREELIST_HPP
