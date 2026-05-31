#include "lightkv/net/Buffer.h"

#include <cassert>
#include <cstring>
#include <string>

namespace {

void append(lightkv::Buffer& buffer, const char* text) {
    buffer.append(text, std::strlen(text));
}

}  // namespace

int main() {
    {
        lightkv::Buffer buffer;
        append(buffer, "abc");
        assert(buffer.size() == 3);
        assert(!buffer.hasLine());
        buffer.clear();
        assert(buffer.size() == 0);
    }

    {
        lightkv::Buffer buffer;
        append(buffer, "PING\n");
        assert(buffer.hasLine());
        assert(buffer.popLine() == "PING");
        assert(!buffer.hasLine());
        assert(buffer.size() == 0);
    }

    {
        lightkv::Buffer buffer;
        append(buffer, "PING\r\n");
        assert(buffer.hasLine());
        assert(buffer.popLine() == "PING");
    }

    {
        lightkv::Buffer buffer;
        append(buffer, "PING\nGET name\n");
        assert(buffer.hasLine());
        assert(buffer.popLine() == "PING");
        assert(buffer.hasLine());
        assert(buffer.popLine() == "GET name");
        assert(!buffer.hasLine());
    }

    {
        lightkv::Buffer buffer;
        append(buffer, "SET name");
        assert(!buffer.hasLine());
        append(buffer, " yifei\n");
        assert(buffer.hasLine());
        assert(buffer.popLine() == "SET name yifei");
    }

    return 0;
}

