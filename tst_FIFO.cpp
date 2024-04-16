#include <iostream>
#include <stdexcept>

class FIFOQueue {
private:
    static const int MAX_NUM = 8;
    int arr[MAX_NUM];
    int front;
    int rear;
    int count;

public:
    FIFOQueue() : front(0), rear(0), count(0) {}

    bool isEmpty() const {
        return count == 0;
    }

    bool isFull() const {
        return count == MAX_NUM;
    }

    void enqueue(int element) {
        if (isFull()) {
            throw std::overflow_error("Queue is full");
        }

        arr[rear] = element;
        rear = (rear + 1) % MAX_NUM;
        ++count;
    }

    int dequeue() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }

        int element = arr[front];
        front = (front + 1) % MAX_NUM;
        --count;
        return element;
    }

    int peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        return arr[front];
    }

    void display() const {
        std::cout << "Queue: ";
        for (int i = 0; i < count; ++i) {
            std::cout << arr[(front + i) % MAX_NUM] << " ";
        }
        std::cout << std::endl;
    }
};

int main() {
    FIFOQueue q;

    try {
        q.enqueue(1);
        q.enqueue(2);
        q.enqueue(3);
        q.enqueue(4);
        q.enqueue(5);
        q.enqueue(6);
        q.enqueue(7);
        q.enqueue(8);
        q.display();

        std::cout << "Dequeued: " << q.dequeue() << std::endl;
        q.display();
        q.enqueue(9);
        q.display();

        std::cout << "Peek: " << q.peek() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
