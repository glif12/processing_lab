#ifndef PARSING_H
#define PARSING_H
#define PATH_LENGTH 256

#include "../include/functions.h"

struct Arguments {
    char templateFile[PATH_LENGTH];
    char outputFile[PATH_LENGTH];
    bool isOutput; // has out in output.txt
    char dataFile[PATH_LENGTH];
};

enum ParserState {
    STATE_NORMAL,      // text
    STATE_OPEN_BRACE1, // find {
    STATE_OPEN_BRACE2, // start key (second '{')
    STATE_IN_TAG,      // part of key
    STATE_CLOSE_BRACE1 // end of key (first '}')
};

struct StreamParser {
    ParserState state;
    char variableName[CSTRING_SIZE];
    int varNamePos;
    HashMap* dataMap;
    FILE* outputFile;
};

void initStreamParser(StreamParser* parser, HashMap* dataMap, FILE* outputFile);
int loadDataFile(const char* filename, HashMap* map);
int parseKeyValueLine(const char* line, char* key, char* value, int maxLen);
bool processChar(StreamParser* parser, char c);
bool processTemplateStream(const char* templatePath, HashMap* dataMap, const char* outputPath);
int parseArguments(int argc, char* argv[], Arguments& args);

#endif