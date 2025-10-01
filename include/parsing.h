#ifndef PARSING_H
#define PARSING_H

#include "../include/functions.h"

int loadDataFile(const char* filename, HashMap* map);
int parseKeyValueLine(const char* line, char* key, char* value, int maxLen);

#endif