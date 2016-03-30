#include "io.hpp"

std::string binary_read_string(std::ifstream& stream, int n)
{
    char* buffer = new char[n+1];
    stream.read(buffer, n);
    return std::string(buffer);
}

std::ostream& binary_write_string(std::ofstream& stream, const std::string& value)
{
    return stream.write(value.c_str(), value.length());
}
