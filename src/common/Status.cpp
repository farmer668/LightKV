#include "lightkv/common/Status.h"

#include <utility>

namespace lightkv {

Status::Status(StatusCode code, std::string message)
    : code_(code), message_(std::move(message)) {}

Status Status::OK() {
    return Status(StatusCode::OK, "");
}

Status Status::NotFound(const std::string& message) {
    return Status(StatusCode::NotFound, message);
}

Status Status::InvalidArgument(const std::string& message) {
    return Status(StatusCode::InvalidArgument, message);
}

Status Status::Error(const std::string& message) {
    return Status(StatusCode::Error, message);
}

bool Status::ok() const {
    return code_ == StatusCode::OK;
}

StatusCode Status::code() const {
    return code_;
}

const std::string& Status::message() const {
    return message_;
}

std::string Status::toString() const {
    std::string code_text;
    switch (code_) {
        case StatusCode::OK:
            code_text = "OK";
            break;
        case StatusCode::NotFound:
            code_text = "NotFound";
            break;
        case StatusCode::InvalidArgument:
            code_text = "InvalidArgument";
            break;
        case StatusCode::Error:
            code_text = "Error";
            break;
    }

    if (message_.empty()) {
        return code_text;
    }

    return code_text + ": " + message_;
}

}  // namespace lightkv

