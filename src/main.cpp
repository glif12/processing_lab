#include "../include/functions.h"
#include "../include/parsing.h"
#include <iostream>

int main(int argc, char* argv[]) {

    // init all 
    HashMap dataMap;
    mapInit(&dataMap);

    Arguments args;
    parseArguments(argc,argv,args);

    // load data file and wirte outpuut file 
    if (loadDataFile(args.dataFile, &dataMap) == 0) 
    {
        switch (args.isOutput) {

            case 1:
                if (processTemplateStream(args.templateFile, &dataMap, args.outputFile)) 
                {
                    std::cout << "Success: Template processed. Check " << args.outputFile << '\n';
                    return 0;
                }

            default: 
                if (processTemplateStream(args.templateFile, &dataMap,"output.txt")) 
                {
                    std::cout << "Success: Template processed. Check output.txt" << '\n';
                    return 0;
                }
                break;
        }
    }
    
    else 
    {
    std::cerr << "Error: Processing failed Erroe code: 3" << '\n';
    return 3;
    }
    
    return 0;
}