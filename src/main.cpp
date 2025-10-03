#include "../include/functions.h"
#include "../include/parsing.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // init all 
    HashMap dataMap;
    mapInit(&dataMap);

    Arguments args;
    int parseResult = parseArguments(argc, argv, args);
    
    if (parseResult != 0) {
        return parseResult; // возврат кода ошибки из парсера
    }

    // load data file
    int loadResult = loadDataFile(args.dataFile, &dataMap);
    if (loadResult != 0) {
        std::cerr << "Error: Failed to load data file. Error code: " << loadResult << '\n';
        return loadResult;
    }

    bool success = false;
    if (args.isOutput) {
        success = processTemplateStream(args.templateFile, &dataMap, args.outputFile);
    } else {
        success = processTemplateStream(args.templateFile, &dataMap, nullptr);
    }

    if (success) {
        if (args.isOutput) {
            std::cout << "Success: Template processed. Check " << args.outputFile << '\n';
        } else {
            std::cout << "Success: Template processed to console" << '\n';
        }
        return 0;
    } else {
        std::cerr << "Error: Template processing failed. Error code: 3" << '\n';
        return 3;
    }
}