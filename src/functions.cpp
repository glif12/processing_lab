#include "../include/functions.h"
#include <stdio.h>
#include <cstdio>

// создание индекса для ключа 
unsigned long hashFunction(const char* string) { // based on djb2 
    unsigned long hash = 5381;
    int c;

    while(1) {   
        c = *string; 
        string++;

        if (c == 0) break;
        hash = ((hash << 5) + hash) ^ c;
    }
    hash = hash%TABLE_SIZE;
    return hash;
}

int findIndex(HashMap* map, const char* key) { 
    unsigned long index = hashFunction(key);
    unsigned long startIndex = index;
    int firstDeleted = -1;

    while (map->mapCells[index].state != EMPTY) {
        if (map->mapCells[index].state == RESERVED && isEqual(map->mapCells[index].key, key)) {
            return index;
        }

        if (map->mapCells[index].state == DELETED && firstDeleted == -1) {
            firstDeleted = index;
        }

        index = (index + 1) % TABLE_SIZE; 
        if (index == startIndex) break; 
    }
    
    return (firstDeleted != -1) ? firstDeleted : index;
}

bool insertMap(HashMap* map, const char* key, const char* value) {
    if (map->size >= TABLE_SIZE) { 
        return false;
    }

    if (strLen(key) == 0 || strLen(value) == 0) { 
        return false;
    }

    if (strLen(value) >= CSTRING_SIZE || strLen(key) >= CSTRING_SIZE) {
        return false;
    }

    int index = findIndex(map, key);

    if (map->mapCells[index].state == RESERVED && isEqual(map->mapCells[index].key, key)) {
        strCopy(map->mapCells[index].value, value);
        return true;
    }

    strCopy(map->mapCells[index].key, key);
    strCopy(map->mapCells[index].value, value);
    map->mapCells[index].state = RESERVED;
    map->size++;

    return true;
}

void mapInit(HashMap* map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->mapCells[i].state = EMPTY;
        map->mapCells[i].key[0] = '\0';
        map->mapCells[i].value[0] = '\0';
    } 
    map->size = 0;
    map->capacity = TABLE_SIZE;
}

bool hashmapFind(HashMap* map, const char* key, char* out_value) {
    int index = findIndex(map, key);
    
    if (index != -1 && 
        map->mapCells[index].state == RESERVED && 
        isEqual(map->mapCells[index].key, key)) {
        strCopy(out_value, map->mapCells[index].value);
        return true;
    }
    
    return false;
}
    
// длина строки 
int strLen(const char* str) {
    size_t i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}
    
// пропуск пробелов в начале строки 
const char* skipSpaces(const char* str) {
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n') {
        str++;
    }
    return str;
}
    
// обрезка пробелов в конце
void trimRight(char* str) {
    int len = strLen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

// копирование до N 
char* strCopyN(char* dest, const char* src, int n) {
    int i = 0;
    while (i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

// поиск подстроки 
const char* strFind(const char* str, const char* substr) {
    int subLen = strLen(substr);
    if (subLen == 0) return str;
    
    for (int i = 0; str[i] != '\0'; i++) {
        bool found = true;
        for (int j = 0; j < subLen; j++) {
            if (str[i + j] != substr[j]) {
                found = false;
                break;
            }
        }
        if (found) return &str[i];
    }
    return nullptr;
}

// равны ли 
bool isEqual(const char* str1,const char* str2) {
    int i = 0;

    while(str1[i] != '\0' && str2[i] != '\0') 
    {
        if (str1[i] != str2[i]) return false;
        i++;
    }

    return str1[i] == '\0' && str2[i] == '\0';
}

// полная копия строки 
char* strCopy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}