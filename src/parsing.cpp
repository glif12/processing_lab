#include "../include/parsing.h"

int parseKeyValueLine(const char* line, char* key, char* value, int maxLen) {
    // скип пробелов
    line = skipSpaces(line);
    
    if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) {
        return -1; // skip line
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
                std::cerr << "Error on line " << lineNumber << ": failed to add pair to hash table" << '\n';
                fclose(file);
                return 3; 
            }
            loadCount++;
            std::cout << "Loaded: " << key << " = " << value << '\n';
        } else if (result == -1) {
            continue;
        } else {
            std::cerr << "Error on line " << lineNumber << ": invalid data format" << 
            " Error code: 3"<< '\n';
            fclose(file);
            return 3;
        }
    }
    std::cout << '\n';

    fclose(file);
    return 0;
}

// обработка файла в потоке 
int processChar(StreamParser* parser, char c) {
    switch (parser->state) {
        case STATE_NORMAL:
            if (c == '{') {
                parser->state = STATE_OPEN_BRACE1;
            } else {
                fputc(c, parser->outputFile);
            }
            break;
            
        case STATE_OPEN_BRACE1:
            if (c == '{') {
                parser->state = STATE_OPEN_BRACE2;
                parser->varNamePos = 0;
                parser->variableName[0] = '\0';
            } else {
                // это была одиночная {, а не начало метки
                fputc('{', parser->outputFile);
                fputc(c, parser->outputFile);
                parser->state = STATE_NORMAL;
            }
            break;
            
        case STATE_OPEN_BRACE2:
            if (c == '}') {
                // обработка пустой метки
                parser->state = STATE_CLOSE_BRACE1;
            } else if (c == '{') {
                // три подряд {{{ - синтаксическая ошибка
                std::cerr << "Error: triple '{{{' is not allowed. Error code: 4" << '\n';
                return 4;
            } else {
                parser->state = STATE_IN_TAG;
                // сбор информации о ключе
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = c;
                    parser->variableName[parser->varNamePos] = '\0';
                }
            }
            break;
            
        case STATE_IN_TAG:
            if (c == '}') {
                parser->state = STATE_CLOSE_BRACE1;
            } else if (c == '{') {
                // открывающая скобка внутри метки - синтаксическая ошибка
                std::cerr << "Error: '{' inside variable tag is not allowed. Error code: 4" << '\n';
                return 4;
            } else if (c == ' ') {
                // игнор пробелов в конце
                if (parser->varNamePos > 0 && parser->variableName[parser->varNamePos - 1] != ' ') {
                    // но только если это не первый пробел после имени
                }
            } else {
                // превращение в cstring
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = c;
                    parser->variableName[parser->varNamePos] = '\0';
                } else {
                    // слишком длинное имя переменной
                    std::cerr << "Error: variable name too long. Error code: 5" << '\n';
                    return 5;
                }
            }
            break;
            
        case STATE_CLOSE_BRACE1:
            if (c == '}') {
                // завершение метки
                parser->state = STATE_NORMAL;
                
                trimRight(parser->variableName);
                const char* cleanedName = skipSpaces(parser->variableName);
                
                // проверка, что переменная не пустая
                if (strLen(cleanedName) == 0) {
                    std::cerr << "Error: empty variable name in template. Error code: 5" << '\n';
                    return 5;
                }
                
                // ищем значение в HashMap
                char value[CSTRING_SIZE];
                if (hashmapFind(parser->dataMap, cleanedName, value)) {
                    fputs(value, parser->outputFile);
                } else {
                    std::cerr << "Error: variable '" << cleanedName << "' not found. Error code: 1" << '\n';
                    return 1;
                }
            } else {
                // Это была одиночная }, а не конец метки
                // возвращаемся в состояние IN_TAG и добавляем } к имени
                parser->state = STATE_IN_TAG;
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = '}';
                    parser->variableName[parser->varNamePos] = '\0';
                }
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = c;
                    parser->variableName[parser->varNamePos] = '\0';
                } else {
                    std::cerr << "Error: variable name too long. Error code: 5" << '\n';
                    return 5;
                }
            }
            break;
    }
    
    return 0;
}

// обработка/запись файлов в потоке 
bool processTemplateStream(const char* templatePath, HashMap* dataMap, const char* outputPath) {
    FILE* templateFile = fopen(templatePath, "r");
    if (!templateFile) {
        std::cerr << "Error: failed to open template file '" << templatePath << "'" << std::endl;
        return false;
    }
    
    FILE* outputFile;
    if (outputPath && strLen(outputPath) > 0) {
        outputFile = fopen(outputPath, "w");
        if (!outputFile) {
            std::cerr << "Error: failed to open output file '" << outputPath << "'" << std::endl;
            fclose(templateFile);
            return false;
        }
    } else {
        outputFile = stdout; // вывод в консоль
    }
    
    StreamParser parser;
    initStreamParser(&parser, dataMap, outputFile);
    
    char buffer[1024];
    bool success = true;
    int result = 0;
    
    while (fgets(buffer, sizeof(buffer), templateFile)) {
        for (int i = 0; buffer[i] != '\0'; i++) {
            result = processChar(&parser, buffer[i]);
            if (result != 0) {
                success = false;
                break;
            }
        }
        if (!success) break;
    }
    
    // проверка на незакрытую метку в конце файла
    if (success && parser.state != STATE_NORMAL) {
        std::cerr << "Error: unclosed tag at the end of file. Error code: 4" << std::endl;
        success = false;
    }
    
    fclose(templateFile);
    if (outputPath && strLen(outputPath) > 0) {
        fclose(outputFile);
    }
    
    return success;
}

// инициализация парсера 
void initStreamParser(StreamParser* parser, HashMap* dataMap, FILE* outputFile) {
    parser->state = STATE_NORMAL;
    parser->varNamePos = 0;
    parser->variableName[0] = '\0';
    parser->dataMap = dataMap;
    parser->outputFile = outputFile;
}

int parseArguments(int argc, char* argv[], Arguments& args) {
    args.templateFile[0] = '\0';
    args.dataFile[0] = '\0';
    args.outputFile[0] = '\0';
    args.isOutput = false;
    
    for (int i = 1; i < argc; i++) {
        if (strFind(argv[i], "--template=") == argv[i]) {
            strCopyN(args.templateFile, argv[i] + 11, PATH_LENGTH-1);
            args.templateFile[PATH_LENGTH-1] = '\0';
        }
        else if (isEqual(argv[i], "-t") && i + 1 < argc) {
            strCopyN(args.templateFile, argv[i + 1], PATH_LENGTH-1);
            args.templateFile[PATH_LENGTH-1] = '\0';
            i++; // skip next arg, cause it's value of -t 
        }
        else if (strFind(argv[i], "--data=") == argv[i]) {
            strCopyN(args.dataFile, argv[i] + 7, PATH_LENGTH-1);
            args.dataFile[PATH_LENGTH-1] = '\0';
        }
        else if (isEqual(argv[i], "-d") && i + 1 < argc) {
            strCopyN(args.dataFile, argv[i + 1], PATH_LENGTH-1);
            args.dataFile[PATH_LENGTH-1] = '\0';
            i++;  
        }
        else if (strFind(argv[i], "--output=") == argv[i]) {
            strCopyN(args.outputFile, argv[i] + 9, PATH_LENGTH-1);
            args.outputFile[PATH_LENGTH-1] = '\0';
            args.isOutput = true;
        }
        else if (isEqual(argv[i], "-o") && i + 1 < argc) {
            strCopyN(args.outputFile, argv[i + 1], PATH_LENGTH-1);
            args.outputFile[PATH_LENGTH-1] = '\0';
            args.isOutput = true;
            i++;
        }
        else {
            std::cerr << "Error: unknown argument '" << argv[i] << "'" << '\n';
            return 2;
        }
    }
    
    // check required arguments
    if (args.templateFile[0] == '\0' || args.dataFile[0] == '\0') {
        std::cerr << "Error: required arguments are missing " 
                  << "Error code: 2" << '\n';
        return 2;
    }
    return 0;
}