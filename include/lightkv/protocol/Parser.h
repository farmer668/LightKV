#pragma once

#include "lightkv/protocol/Command.h"

#include <string>

namespace lightkv {

class Parser {
public:
    Command parseLine(const std::string& line) const;
};

}  // namespace lightkv

