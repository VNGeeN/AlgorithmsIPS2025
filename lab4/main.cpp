#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

class MinHeap {
private:
    std::vector<int> heap;

    // Вспомогательные функции
    void siftDown(int index) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < heap.size() && heap[left] < heap[smallest]) {
            smallest = left;
        }
        if (right < heap.size() && heap[right] < heap[smallest]) {
            smallest = right;
        }

        if (smallest != index) {
            std::swap(heap[index], heap[smallest]);
            siftDown(smallest);
        }
    }

    void siftUp(int index) {
        while (index > 0 && heap[index] < heap[(index - 1) / 2]) {
            std::swap(heap[index], heap[(index - 1) / 2]);
            index = (index - 1) / 2;
        }
    }

public:
    // 1. Построение кучи из массива
    void buildHeap(const std::vector<int>& data) {
        heap = data;
        for (int i = heap.size() / 2 - 1; i >= 0; i--) {
            siftDown(i);
        }
    }

    // 2. Добавление элемента
    void insert(int value) {
        heap.push_back(value);
        siftUp(heap.size() - 1);
    }

    // 3. Удаление элемента по значению (первого найденного)
    void removeByValue(int value) {
        auto it = std::find(heap.begin(), heap.end(), value);
        if (it == heap.end()) return;

        int index = it - heap.begin();
        heap[index] = heap.back();
        heap.pop_back();

        if (index < heap.size()) {
            siftDown(index);
            siftUp(index);
        }
    }

    // 4. Изменение значения элемента по индексу
    void changeValue(int index, int newValue) {
        if (index < 0 || index >= heap.size()) return;
        int oldValue = heap[index];
        heap[index] = newValue;

        if (newValue < oldValue) {
            siftUp(index);
        } else {
            siftDown(index);
        }
    }

    // 5. Вывод кучи в виде массива и древовидной структуры
    void display() const {
        // Вывод в виде массива
        std::cout << "Куча (массив): ";
        for (int val : heap) {
            std::cout << val << " ";
        }
        std::cout << "\n";

        // Вывод в виде дерева (по уровням)
        std::cout << "Древовидное представление:\n";
        int levelSize = 1;
        int count = 0;
        for (int i = 0; i < heap.size(); i++) {
            std::cout << heap[i] << " ";
            count++;
            if (count == levelSize) {
                std::cout << "\n";
                levelSize *= 2;
                count = 0;
            }
        }
        std::cout << "\n";
    }

    // Вспомогательная функция для получения размера
    int size() const {
        return heap.size();
    }

    // Вспомогательная функция для получения индекса по значению
    int findIndex(int value) const {
        auto it = std::find(heap.begin(), heap.end(), value);
        return (it != heap.end()) ? (it - heap.begin()) : -1;
    }
};

// Чтение чисел из файла
std::vector<int> readNumbersFromFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<int> numbers;
    std::string line;

    if (file.is_open()) {
        std::getline(file, line);
        std::stringstream ss(line);
        int num;
        while (ss >> num) {
            numbers.push_back(num);
        }
        file.close();
    } else {
        std::cerr << "Не удалось открыть файл: " << filename << "\n";
    }
    return numbers;
}

int main() {
    MinHeap heap;
    std::string filename;
    int choice, value, index;

    std::cout << "Введите имя файла с числами: ";
    std::cin >> filename;

    // 1. Построение кучи из файла
    std::vector<int> initialData = readNumbersFromFile(filename);
    if (initialData.empty()) {
        std::cout << "Файл пуст или не найден. Начинаем с пустой кучи.\n";
    } else {
        heap.buildHeap(initialData);
        std::cout << "Куча построена.\n";
        heap.display();
    }

    do {
        std::cout << "\nМеню:\n";
        std::cout << "1. Добавить элемент\n";
        std::cout << "2. Удалить элемент по значению\n";
        std::cout << "3. Изменить значение элемента по индексу\n";
        std::cout << "4. Показать кучу\n";
        std::cout << "5. Выход\n";
        std::cout << "Выбор: ";
        std::cin >> choice;

        switch (choice) {
            case 1: // Добавление элемента
                std::cout << "Введите значение для добавления: ";
                std::cin >> value;
                heap.insert(value);
                std::cout << "Элемент добавлен.\n";
                heap.display();
                break;

            case 2: // Удаление элемента по значению
                std::cout << "Введите значение для удаления: ";
                std::cin >> value;
                heap.removeByValue(value);
                std::cout << "Элемент удален (если существовал).\n";
                heap.display();
                break;

            case 3: // Изменение значения по индексу
                std::cout << "Введите индекс элемента (0-" << heap.size() - 1 << "): ";
                std::cin >> index;
                if (index >= 0 && index < heap.size()) {
                    std::cout << "Введите новое значение: ";
                    std::cin >> value;
                    heap.changeValue(index, value);
                    std::cout << "Значение изменено.\n";
                } else {
                    std::cout << "Неверный индекс!\n";
                }
                heap.display();
                break;

            case 4: // Отображение кучи
                heap.display();
                break;

            case 5: // Выход
                std::cout << "Выход...\n";
                break;

            default:
                std::cout << "Неверный выбор!\n";
        }
    } while (choice != 5);

    return 0;
}