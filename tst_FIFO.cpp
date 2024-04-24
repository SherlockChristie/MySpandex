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

    void envector(int element) {
        if (isFull()) {
            throw std::overflow_error("Queue is full");
        }

        arr[rear] = element;
        rear = (rear + 1) % MAX_NUM;
        ++count;
    }

    int devector() {
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
        q.envector(1);
        q.envector(2);
        q.envector(3);
        q.envector(4);
        q.envector(5);
        q.envector(6);
        q.envector(7);
        q.envector(8);
        q.display();

        std::cout << "Devectord: " << q.devector() << std::endl;
        q.display();
        q.envector(9);
        q.display();

        std::cout << "Peek: " << q.peek() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
