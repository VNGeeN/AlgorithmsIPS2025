#ifndef CUSTOM_STACK_H
#define CUSTOM_STACK_H

#include <stdexcept>

template<typename T>
class CustomStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    Node* topNode;
    size_t stackSize;
    
public:
    CustomStack() : topNode(nullptr), stackSize(0) {}
    
    ~CustomStack() {
        clear();
    }
    
    void push(const T& value) {
        Node* newNode = new Node(value);
        newNode->next = topNode;
        topNode = newNode;
        stackSize++;
    }
    
    T pop() {
        if (isEmpty()) {
            throw std::runtime_error("Stack underflow");
        }
        
        Node* temp = topNode;
        T value = temp->data;
        topNode = topNode->next;
        delete temp;
        stackSize--;
        
        return value;
    }
    
    T& top() const {
        if (isEmpty()) {
            throw std::runtime_error("Stack is empty");
        }
        return topNode->data;
    }
    
    bool isEmpty() const {
        return topNode == nullptr;
    }
    
    size_t size() const {
        return stackSize;
    }
    
    void clear() {
        while (!isEmpty()) {
            pop();
        }
    }
    
    CustomStack(const CustomStack&) = delete;
    CustomStack& operator=(const CustomStack&) = delete;
};

#endif