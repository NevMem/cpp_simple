#pragma once

#include <string>
#include <optional>


namespace cmd {

bool hasValueInArgs(int argc, char** argv, const std::string& value);
std::optional<std::string> getValue(int argc, char** argv, const std::string& paramName);

}
