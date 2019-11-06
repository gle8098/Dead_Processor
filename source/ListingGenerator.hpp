#pragma once

#include <cstdio>

class ListingGenerator {
    FILE* outstream;
    int offset = 0;

public:
    explicit ListingGenerator(FILE* outstream);

    // Все функции Print... возвращают количество прочитанных байт, или -1 в случае ошибки

    // Печатает заголовок .friday, может быть вызвана, только если GetOffset() == 0
    int PrintHeader(const char* code, int code_length);
    int PrintInstruction(const char* code, int code_length);

    // Возвращает текущее смещение относительно начала файла,
    // то есть суммарное количество прочитанных байт
    int GetOffset() const;
};


