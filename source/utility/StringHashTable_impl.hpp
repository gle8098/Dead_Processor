
// class StringHashTable //

template <typename T>
bool StringHashTable<T>::CheckLoadFactor() {
    return table.size() * maxLoadFactor > size;
}

template <typename T>
void StringHashTable<T>::RehashIfNeeded() {
    if (CheckLoadFactor())
        return;

    // Создаем новую пустую таблицу с в два раза увеличенным capacity.
    std::vector<ElemType> oldTable = std::move(table);
    table = std::vector<ElemType>(2 * oldTable.size());
    hashFunc = HashFunc(table.size());
    size = 0;

    // Заполняем новую таблицу элементами из старой
    for (auto &elem : oldTable) {
        if (elem.state == ElemType::State::occupied) {
            Insert(elem.key, elem.value);
        }
    }
}

template <typename T>
std::pair<typename StringHashTable<T>::ElemType *, typename StringHashTable<T>::ElemType *> StringHashTable<T>
        ::FindCell(const std::string &str) {
    ElemType* occupiedElem = nullptr;
    ElemType* vacantElem = nullptr;

    size_t index = hashFunc(str);
    for (size_t i = 0; i < table.size(); ++i) {
        ElemType& elem = table[index];

        bool exitFunc = false;
        switch (elem.state) {
            case ElemType::State::nil:
                if (vacantElem == nullptr) {
                    vacantElem = &elem;
                }
                exitFunc = true; // Последовательность закончена, все последующие элементы имеют state = nil
                break;

            case ElemType::State::occupied:
                if (elem.key == str) {
                    occupiedElem = &elem;
                }
                exitFunc = vacantElem != nullptr; // Мы не должны выходить до того, как найдем свободную ячейку
                break;

            case ElemType::State::deleted:
                if (vacantElem == nullptr) {
                    vacantElem = &elem;
                }
                break;
        }
        if (exitFunc) break;
        index = hashFunc(index, i + 1);
    }

    return std::make_pair(occupiedElem, vacantElem);
}

template <typename T>
StringHashTable<T>::StringHashTable(double maxLoadFactor)
        : table(8)
        , hashFunc(table.size())
        , maxLoadFactor(maxLoadFactor)
{}

template <typename T>
bool StringHashTable<T>::Insert(const std::string &key, const T& value) {
    RehashIfNeeded();

    auto cells = FindCell(key);
    if (cells.first != nullptr) {
        return false;
    }
    ++size;
    cells.second->key = key;
    cells.second->state = ElemType::State::occupied;
    cells.second->value = value;
    return true;
}

template <typename T>
T* StringHashTable<T>::Find(const std::string &key) {
    auto cells = FindCell(key);
    return (cells.first != nullptr) ? &cells.first->value : nullptr;
}

template <typename T>
bool StringHashTable<T>::Delete(const std::string &key) {
    auto cells = FindCell(key);
    if (cells.first == nullptr)
        return false;
    cells.first->state = ElemType::State::deleted;
    --size;
    return true;
}


// class HastTable::HashFunc //

template <typename T>
size_t StringHashTable<T>::HashFunc::operator()(const std::string &str) {
    size_t hash = 0;
    for (char letter : str) {
        hash = (hash * factor + static_cast<size_t>(letter)) % module;
    }
    return hash;
}

template <typename T>
size_t StringHashTable<T>::HashFunc::operator()(size_t prevHash, size_t nextIt) {
    return (prevHash + nextIt) % module;
}

template <typename T>
StringHashTable<T>::HashFunc::HashFunc(size_t module)
        : module(module) // module должно быть степенью двойки
{}
