#pragma once

#include <string>
#include <vector>

// Хеш-таблица, хранящая ключи std::string и значения T. Построена на std::vector.
template <typename T>
class StringHashTable {
private:
    // Класс для вычисления хэшей. Используется квадратичное преобразование с c1 = c2 = 1/2.
    class HashFunc {
    private:
        size_t module;
        size_t factor = 100001;
    public:
        explicit HashFunc(size_t module);
        // Возвращает первый элемент в последовательности проб для str.
        size_t operator()(const std::string& str);
        // Возвращает следующий элемент в последовательности проб, если хэш предыдущего значения -- prevHash,
        // а номер следующего элемента -- nextIt.
        size_t operator()(size_t prevHash, size_t nextIt);
    };

    struct ElemType {
        enum class State {nil, occupied, deleted};

        State state = State::nil;
        std::string key;
        T value;
    };

    std::vector<ElemType> table;
    HashFunc hashFunc;
    size_t size = 0;
    const double maxLoadFactor;

    // Возвращает true, если load factor не превысил максимальное значение
    inline bool CheckLoadFactor();

    // Увеличивает размер таблицы в два раза, если load factor превысил максимальное значение
    void RehashIfNeeded();

    // Для данного str находит и возвращает соответственно:
    // (1) ячейку из table со значением str, если такая есть, или nullptr
    // (2) первую ячейку из последовательности, на место которой можно вставить новое значение
    std::pair<ElemType*, ElemType*> FindCell(const std::string& str);

public:
    explicit StringHashTable(double maxLoadFactor = 0.75);

    // Добавляет элемент в таблицу. Возвращает false, если элемент был уже добавлен, иначе true.
    bool Insert(const std::string& key, const T& value);

    // Возвращает указатель на значение по ключу, если ключ содержится в таблице. Иначе nullptr
    T* Find(const std::string& key);

    // Удаляет элемент. Если элемент не содержится в таблице, возвращает false, иначе true.
    bool Delete(const std::string& key);
};

#include "StringHashTable_impl.hpp"
