#include <iostream>
#include "unrolled_list.h"

int main() {
    // Создаём объект unrolled_list с максимальным размером узла 5
    unrolled_list<int, 5> ul;

    // Добавляем элементы с помощью push_back и push_front.
    for (int i = 1; i <= 10; ++i) {
        ul.push_back(i);
    }
    ul.push_front(0);

    // Выводим содержимое контейнера.
    std::cout << "Elements in unrolled_list:" << std::endl;
    for (auto it = ul.begin(); it != ul.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // Проверяем pop_back и pop_front.
    ul.pop_back();   // Удаляем последний элемент.
    ul.pop_front();  // Удаляем первый элемент.

    std::cout << "After pop_back and pop_front:" << std::endl;
    for (auto it = ul.begin(); it != ul.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    return 0;
}
