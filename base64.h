#ifndef BASE64_H
#define BASE64_H

#include <tuple>
#include <string>
#include <vector>

class BASE64 {
public:
    static std::string encode(const std::string& str);
    static std::string decode(const std::string& str);
private:
    static const std::string base;

    static std::string encode_part(const std::string& str, int start);
    static std::string decode_part(const std::string& str, int start);
};

#endif
