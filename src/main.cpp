#include "interpreter.h"
#include "error.h"
#include "tokenzier.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <filename> [function] [args...]" << std::endl;
            return 1;
        }
        
        std::string filename = argv[1];
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filename << std::endl;
            return 1;
        }
        
        Interpreter interpreter(file);
        
        if (argc >= 3) {
            std::string function_name = argv[2];
            std::vector<int> args;
            
            for (int i = 3; i < argc; i++) {
                try {
                    args.push_back(std::stoi(argv[i]));
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: Invalid argument '" << argv[i] << "', expected integer" << std::endl;
                    return 1;
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: Argument '" << argv[i] << "' is out of valid integer range" << std::endl;
                    return 1;
                }
            }
            
            int result = interpreter.run(function_name, args);
            std::cout << "Result: " << result << std::endl;
        } else {
            std::cout << "No function specified to run." << std::endl;
        }
        
    } catch (const SyntaxError& e) {
        std::cerr << "Syntax Error: " << e.what() << std::endl;
        return 1;
    } catch (const NameError& e) {
        std::cerr << "Name Error: " << e.what() << std::endl;
        return 1;
    } catch (const RuntimeError& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}