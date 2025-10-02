#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#define TABLE_SIZE 1024 
#define CSTRING_SIZE 100

#include <iostream>
#include <cstdlib>
#include <ctime> 

enum cellState {
    EMPTY,
    RESERVED, 
    DELETED, 
};

struct Cell {
    char key[CSTRING_SIZE];
    char value[CSTRING_SIZE];
    cellState state;
};

struct HashMap {
    Cell mapCells[TABLE_SIZE];
    int size;
    int capacity; 
};

// для мапы 
unsigned long hashFunction(const char* string);
bool insertMap(HashMap* map,const char* key,const char* value);
int findIndex(HashMap* map,const char* key);
void mapInit(HashMap* map);
bool hashmapFind(HashMap* map,const char* key,char* out_value);

// для c-string
int strLen(const char* str);
bool isEqual(const char* str1,const char* str2);
char* strCopy(char* dest, const char* src);
const char* strFind(const char* str, const char* substr);
char* strCopyN(char* dest, const char* src, int n);
const char* skipSpaces(const char* str);
void trimRight(char* str);

#endif