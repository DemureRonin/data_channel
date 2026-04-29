#pragma once

#include <iosfwd>
#include <string>
#include <fstream>

class Logger {
public:
    static void Log(const std::string &filename, const std::string &content) {
        //std::ofstream file(filename, std::ios::app);
       // file << content.c_str() << std::endl;
    }

    static void Clear(const std::string &filename) {
       // std::ofstream file(filename, std::ios::trunc);
    }
};
