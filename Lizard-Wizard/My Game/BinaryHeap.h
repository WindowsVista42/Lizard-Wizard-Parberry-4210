#ifndef BINARYHEAP_H
#define BINARYHEAP_H

#include <vector>
#include "Defines.h"

// Standard Binary Min Heap.
// See an example usage of this Binary Heap in CGame::Pathfind()
// If you want a max heap then flip <'s and >'s.
template <typename T>
class BinaryHeap {
// Sean: This is more or less ripped from my Guo 3600 implementation of a min heap.
    std::vector<T> heap;

    usize Left(usize parent) {
        return parent * 2 + 1;
    }

    usize Right(usize parent) {
        return parent * 2 + 2;
    }

    usize Parent(usize child) {
        return (child - 1) / 2;
    }

    void HeapifyUp(usize index) {
        usize parent;

        while (true) {
            parent = Parent(index);

            if (index != 0 && parent >= 0 && heap[parent] < heap[index]) {
                std::swap(heap[parent], heap[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void HeapifyDown(usize index) {
        usize left, right, largest;

        while (true) {
            left = Left(index);
            right = Right(index);
            largest = index;

            if (left < Size() && heap[left] > heap[index]) {
                largest = left;
            }

            if (right < Size() && heap[right] > heap[index]) {
                largest = right;
            }

            if (largest != index) {
                std::swap(heap[index], heap[largest]);
                index = largest;
            } else {
                break;
            }
        }
    }

public:
    BinaryHeap() = default;

    void Push(T t) {
        heap.push_back(t);
        HeapifyUp(heap.size() - 1);
    }

    T Pop() {
        if (heap.size() == 0) { return T::default(); }

        T head = heap[0];
        heap[0] = heap[heap.size() - 1];
        heap.pop_back();

        HeapifyDown(0);

        return head;
    }

    usize Size() { return heap.size(); }
    void Clear() { heap.clear(); }
};

#endif
