#include "../include/parsing.h"

struct ParsingArgs {
    
};

bool isOption(const char* str) {
    return str[0] == '-';
}

int parseKeyValueLine(const char* line, char* key, char* value, int maxLen) {
    // скип пробелов
    line = skipSpaces(line);
    
    //скип комментариев и выход, если строка == комментарий
    if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) {
        return 5; // в строке нет знака '=', значит строка не валидна
    }
    
    // ищем позицию знака '=' для разбиения строки на пару 
    int equalPos = -1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '=') {
            equalPos = i;
            break;
        }
    }
    
    if (equalPos == -1) { // позиция '=' не найдена -> ошибка 
        std::cerr << "DataError: incorrect input data entry. Error code: 5" << '\n';
        return 5;
    }
    
    // извлекаем ключ (до знака =)
    int keyStart = 0;
    int keyEnd = equalPos - 1;
    
    // пропускаем пробелы в конце ключа
    while (keyEnd >= keyStart && line[keyEnd] == ' ') {
        keyEnd--;
    }
    
    // копируем ключ
    int keyLength = keyEnd - keyStart + 1;
    if (keyLength <= 0 || keyLength >= maxLen) {
        std::cerr << "KeyError: the key length is incorrect. Error code: 5" << '\n';
        return 5;
    }
    
    strCopyN(key, line, keyLength);
    key[keyLength] = '\0'; 
    
    // извлекаем значение (после знака =)
    const char* valueStart = line + equalPos + 1;
    valueStart = skipSpaces(valueStart); // пропускаем пробелы после '='
    
    // находим конец значения (до комментария или конца строки) + считаем длину значени 
    int valueLength = 0;
    while (valueStart[valueLength] != '\0' && 
           valueStart[valueLength] != '#' &&
           !(valueStart[valueLength] == '/' && valueStart[valueLength + 1] == '/') &&
           valueStart[valueLength] != '\r' && 
           valueStart[valueLength] != '\n') { 
        valueLength++; // версия с пробелами в значении, так бы пробел был опорным для break 
    }
    
    // пропускаем пробелы в конце значения
    while (valueLength > 0 && valueStart[valueLength-1] == ' ') {
        valueLength--;
    }
    
    if (valueLength < 0 || valueLength >= maxLen) {
        std::cerr << "ValueError: the value length is incorrect. Error code: 5" << '\n';
        return 5;
    }
    
    strCopyN(value, valueStart, valueLength);
    value[valueLength] = '\0';
    
    return 0;
}

int loadDataFile(const char* filename, HashMap* map) { 

    FILE* file = fopen(filename,"r");
    if (!file) {
        std::cerr << "Error with read file: " << filename << " Error code: 3" << '\n';
        return 3;
    }

    char line[256];
    int lineNumber = 0, loadCount = 0;
    
    while (fgets(line,sizeof(line),file)) {
        lineNumber++;

        char key[CSTRING_SIZE];
        char value[CSTRING_SIZE];
        
        // пропуск пустых строк 
        const char* trimmedLine = skipSpaces(line); 
        if (strLen(trimmedLine) == 0) {
            continue;
        }

        // обрезка строки справа
        char processedLine[256];
        strCopyN(processedLine, line, sizeof(processedLine) - 1);
        processedLine[sizeof(processedLine) - 1] = '\0';
        trimRight(processedLine);

        int result = parseKeyValueLine(processedLine, key, value, CSTRING_SIZE);
        if (result == 0) {
            if (!insertMap(map, key, value)) {
                std::cerr << "Error on line " << lineNumber << ": failed to add pair to hash table" << std::endl;
                fclose(file);
                return 3; 
            }
            loadCount++;
            std::cout << "Loaded: " << key << " = " << value << std::endl;
        } else if (result == 5) {
            continue;
        } else {
            std::cerr << "Error on line " << lineNumber << ": invalid data format" << 
            " Error code: 3"<< '\n';
            fclose(file);
            return 3;
        }
    }

    fclose(file);
    return 0;
}