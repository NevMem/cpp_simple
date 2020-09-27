#include <cmd/cmd_utils.h>


namespace cmd {

namespace {

bool hasEqualSign(const char* str)
{
    while (*str != '\0') {
        if (*str == '=') {
            return true;
        }
        ++str;
    }
    return false;
}

std::string getAfterEqualSign(const char* str)
{
    std::string result;
    bool wasSign = false;
    while (*str != '\0') {
        if (wasSign) {
            result += *str;
        }
        if (*str == '=') {
            wasSign = true;
        }
        ++str;
    }

    return result;
}

}

bool hasValueInArgs(int argc, char** argv, const std::string& value)
{
    for (size_t i = 0; i != argc; ++i) {
        if (argv[i] == value) {
            return true;
        }
    }
    return false;
}

std::optional<std::string> getValue(int argc, char** argv, const std::string& paramName)
{
    for (size_t i = 0; i != argc; ++i) {
        if (hasEqualSign(argv[i])) {
            return std::optional<std::string>(getAfterEqualSign(argv[i]));
        }
    }
    return std::nullopt;
}
    
}

