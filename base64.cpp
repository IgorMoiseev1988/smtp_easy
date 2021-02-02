#include "base64.h"

#include <sstream> // std::stringstream
#include <cstdint> // std::uint32_t

const std::string BASE64::base = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string BASE64::encode(const std::string& str) {
    std::stringstream out;
    int length = str.size();
    for (int start = 0; start < length; start += 3) {
        out << encode_part(str, start);
    }
    return out.str();
}

std::string BASE64::decode(const std::string & str) {
    std::stringstream out;
    int length = str.size();
    for (int start = 0; start < length; start += 4) {
        out << decode_part(str, start);
    }
    return out.str();
}

std::string BASE64::encode_part(const std::string& str, int start) {
    std::stringstream out;
    int encode_length = (str.size() - start < 3) ? str.size() - start : 3;
    int buff = 0;
    for (int S = 0, shift = 16; S < encode_length; ++S, shift -= 8) {
        buff += static_cast<int>(str[start + S]) << shift;
    }

    for(int S = 0, MASK = 0xFC'00'00, shift = 18; S <= encode_length; ++S, MASK = MASK >> 6, shift -= 6) {
        int R = buff & MASK;
        R = R >> shift;
        out << base[R];
    }

    while(encode_length++ < 3) {
        out << '=';
    }

    return out.str();
}

std::string BASE64::decode_part(const std::string& str, int start) {
    std::stringstream out;
    int buff = 0;
    int decode_length = (str.size() - start < 4) ? str.size() : 4;
    for (int S = 0, shift = 18; S < decode_length; ++S, shift -= 6) {
        char c = str[S + start];
        if (c == '=') {
            decode_length = S;
            break;
        }
        int pos = base.find(c);
        buff += pos << shift;
    }
    if (decode_length == 4) --decode_length;
    for (int N = 0, shift = 16, MASK = 0xFF'00'00; N < decode_length; ++N, shift -= 8, MASK = MASK >> 8) {
        int C = buff & MASK;
        int R = C >> shift;
        out << static_cast<char>(R);
    }

    return out.str();
}
