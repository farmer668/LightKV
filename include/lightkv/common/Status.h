#pragma once

#include <string>

namespace lightkv {

enum class StatusCode {
    OK,
    NotFound,
    InvalidArgument,
    Error
};

class Status {
public:
    static Status OK();
    static Status NotFound(const std::string& message = "");
    static Status InvalidArgument(const std::string& message = "");
    static Status Error(const std::string& message = "");

    bool ok() const;
    StatusCode code() const;
    const std::string& message() const;
    std::string toString() const;

private:
    Status(StatusCode code, std::string message);

    StatusCode code_;
    std::string message_;
};

}  // namespace lightkv

