#pragma once

#include <unordered_map>
#include <unordered_set>

namespace Plaquette {
namespace Types {

// template <typename T> using UnorderedSet = ankerl::unordered_dense::set<T>;
template <typename T> using UnorderedSet = std::unordered_set<T>;
template <typename T, typename V> using UnorderedMap = std::unordered_map<T, V>;

// template <typename T, typename V> using UnorderedMap = emhash5::HashMap<T,
// V>;

}; // namespace Types
}; // namespace Plaquette
