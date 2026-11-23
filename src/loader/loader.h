#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

#include "core/program.h"

class LoadError : public std::runtime_error {
public:
    LoadError(const std::string &message, size_t line)
        : std::runtime_error(line == 0 ? message : "line " + std::to_string(line) + ": " + message), line_(line) {}
    size_t line() const noexcept { return line_; }
private:
    size_t line_;
};

Program load_program(const std::string &path);
