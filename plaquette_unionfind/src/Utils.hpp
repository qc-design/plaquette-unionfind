#pragma once

#include <iostream>
#include <vector>

namespace Plaquette {
namespace Utils {
std::vector<bool> SetXor(std::vector<bool> a, std::vector<bool> b) {
    std::vector<bool> result;
    for (size_t i = 0; i < a.size(); i++) {
        result.push_back(a[i] ^ b[i]);
    }
    return result;
}
}; // namespace Utils
}; // namespace Plaquette
