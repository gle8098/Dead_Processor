#pragma once

namespace BytesHelper {
    template<typename T>
    inline T ReadFromBytes(const char* buffer, int buffer_offset = 0) {
        return *reinterpret_cast<const T*>(buffer + buffer_offset);
    }

    template <typename T>
    inline void WriteBytes(char* buffer, const T& value, int buffer_offset = 0) {
        *reinterpret_cast<T*>(buffer + buffer_offset) = value;
    }

    template <typename T>
    inline const T& BytesAs(const char* bytes, int buffer_offset = 0) {
        return *reinterpret_cast<const T*>(bytes + buffer_offset);
    }
    template <typename T>
    inline T& BytesAs(char* bytes, int buffer_offset = 0) {
        return *reinterpret_cast<T*>(bytes + buffer_offset);
    }

    template <typename T>
    inline const char* AsBytes(const T& value) {
        return reinterpret_cast<const char*>(&value);
    }
};
