#include "../include/functions.h"
#include "../include/parsing.h"
#include <iostream>

enum ParserState {
    STATE_NORMAL,      // Обычный текст
    STATE_OPEN_BRACE1, // Найдена первая {
    STATE_OPEN_BRACE2, // Найдена вторая { (начало метки)
    STATE_IN_TAG,      // Внутри метки {{ ... }}
    STATE_CLOSE_BRACE1 // Найдена первая }
};

struct StreamParser {
    ParserState state;
    char variableName[CSTRING_SIZE];
    int varNamePos;
    HashMap* dataMap;
    FILE* outputFile;
};

void initStreamParser(StreamParser* parser, HashMap* dataMap, FILE* outputFile) {
    parser->state = STATE_NORMAL;
    parser->varNamePos = 0;
    parser->variableName[0] = '\0';
    parser->dataMap = dataMap;
    parser->outputFile = outputFile;
}

// Обрабатывает один символ в потоковом режиме
bool processChar(StreamParser* parser, char c) {
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
                // Это была одиночная {, а не начало метки
                fputc('{', parser->outputFile);
                fputc(c, parser->outputFile);
                parser->state = STATE_NORMAL;
            }
            break;
            
        case STATE_OPEN_BRACE2:
            if (c == ' ') {
                // Пробел после {{ - игнорируем
                break;
            } else if (c == '}') {
                // Случай {{}} - пустая метка
                parser->state = STATE_CLOSE_BRACE1;
            } else {
                parser->state = STATE_IN_TAG;
                // Начинаем собирать имя переменной
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = c;
                    parser->variableName[parser->varNamePos] = '\0';
                }
            }
            break;
            
        case STATE_IN_TAG:
            if (c == '}') {
                parser->state = STATE_CLOSE_BRACE1;
            } else if (c == ' ') {
                // Пробелы в конце имени - игнорируем
                if (parser->varNamePos > 0 && parser->variableName[parser->varNamePos - 1] != ' ') {
                    // Но только если это не первый пробел после имени
                }
            } else {
                // Добавляем символ к имени переменной
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = c;
                    parser->variableName[parser->varNamePos] = '\0';
                } else {
                    // Слишком длинное имя переменной
                    std::cerr << "Error: variable name too long" << std::endl;
                    return false;
                }
            }
            break;
            
        case STATE_CLOSE_BRACE1:
            if (c == '}') {
                // Завершили метку {{ variable }}
                parser->state = STATE_NORMAL;
                
                // Обрезаем пробелы в имени переменной
                trimRight(parser->variableName);
                const char* cleanedName = skipSpaces(parser->variableName);
                
                // Проверяем, что имя переменной не пустое
                if (strLen(cleanedName) == 0) {
                    std::cerr << "Error: empty variable name in template" << std::endl;
                    return false;
                }
                
                // Ищем значение в hashMap
                char value[CSTRING_SIZE];
                if (hashmapFind(parser->dataMap, cleanedName, value)) {
                    fputs(value, parser->outputFile);
                } else {
                    std::cerr << "Error: variable '" << cleanedName << "' not found" << std::endl;
                    return false;
                }
            } else {
                // Это была одиночная }, а не конец метки
                // Возвращаемся в состояние IN_TAG и добавляем } к имени
                parser->state = STATE_IN_TAG;
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = '}';
                    parser->variableName[parser->varNamePos] = '\0';
                }
                if (parser->varNamePos < CSTRING_SIZE - 1) {
                    parser->variableName[parser->varNamePos++] = c;
                    parser->variableName[parser->varNamePos] = '\0';
                } else {
                    std::cerr << "Error: variable name too long" << std::endl;
                    return false;
                }
            }
            break;
    }
    
    return true;
}

// Обрабатывает файл шаблона потоково
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
        outputFile = stdout; // Вывод в консоль
    }
    
    StreamParser parser;
    initStreamParser(&parser, dataMap, outputFile);
    
    char buffer[1024];
    bool success = true;
    
    while (fgets(buffer, sizeof(buffer), templateFile)) {
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (!processChar(&parser, buffer[i])) {
                success = false;
                break;
            }
        }
        if (!success) break;
    }
    
    // Проверяем, что не остались в середине метки
    if (success && parser.state != STATE_NORMAL) {
        std::cerr << "Error: unclosed tag at the end of file" << std::endl;
        success = false;
    }
    
    fclose(templateFile);
    if (outputPath && strLen(outputPath) > 0) {
        fclose(outputFile);
    }
    
    return success;
}

int main() {
    // Минимальная версия без лишних сообщений
    HashMap dataMap;
    mapInit(&dataMap);
    
    // Загрузка данных и обработка шаблона
    if (loadDataFile("../src/data.txt", &dataMap) == 0) {
        if (processTemplateStream("../src/input.txt", &dataMap, "../src/output.txt")) {
            std::cout << "Success: Template processed. Check output.txt" << std::endl;
            return 0;
        }
    }
    
    std::cerr << "Error: Processing failed" << std::endl;
    return 1;
}