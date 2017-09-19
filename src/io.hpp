#ifndef IO_H
#define IO_H

#include <iostream>
#include <fstream>

#include "Logging.hpp"

// http://baptiste-wicht.com/posts/2011/06/write-and-read-binary-files-in-c.html
template<typename T>
std::istream & binary_read(std::istream& stream, T& value)
{
    return stream.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename T>
std::ostream& binary_write(std::ostream& stream, const T& value)
{
    return stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

std::string binary_read_string(std::ifstream& stream, int n);
std::ostream& binary_write_string(std::ofstream& stream, const std::string& value);

bool file_exist(const char *fileName);
void gzip(const std::string filename);

#endif
