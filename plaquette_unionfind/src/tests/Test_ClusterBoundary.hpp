#include "ClusterBoundary.hpp"
#include "DecodingGraph.hpp"
#include "ErrorModels.hpp"
#include "PeelingDecoder.hpp"
#include "StabilizerCode.hpp"
#include "ToricCode.hpp"
#include "UnionFindDecoder.hpp"
#include "Utils.hpp"
#include <catch2/catch.hpp>

using namespace Plaquette;
using namespace Plaquette::ErrorModels;
using namespace Plaquette::Decoders;

TEST_CASE("Test ClusterBoundary size() function", "[ClusterBoundary]") {
    std::vector<int> row = {1, 0, 2, 0, 3};
    ClusterBoundary cb(row, 1, 4);
    REQUIRE(cb.size() == 3);
}

TEST_CASE("Test ClusterBoundary operator[] function", "[ClusterBoundary]") {
    std::vector<int> row = {1, 0, 2, 0, 3};
    ClusterBoundary cb(row, 1, 4);
    REQUIRE(cb[0] == 0);
    REQUIRE(cb[1] == 2);
    REQUIRE(cb[2] == 0);
}

TEST_CASE("Test ClusterBoundary begin() and end() functions",
          "[ClusterBoundary]") {
    std::vector<int> row = {1, 0, 2, 0, 3};
    ClusterBoundary cb(row, 1, 4);
    auto it = cb.begin();
    REQUIRE(*it == 0);
    it++;
    REQUIRE(*it == 2);
    it++;
    REQUIRE(*it == 0);
    it++;
    REQUIRE(it == cb.end());
}

TEST_CASE("Test ClusterBoundaries Add() and GetBoundary() functions",
          "[ClusterBoundaries]") {
    std::vector<size_t> clusters = {0, 2, 4};
    ClusterBoundaries cbs(clusters, 6, 2);
    cbs.Add(0, 1);
    cbs.Add(0, 2);
    cbs.Add(2, 3);
    auto cb0 = cbs.GetBoundary(0);
    auto cb2 = cbs.GetBoundary(2);
    REQUIRE(cb0.size() == 2);
    REQUIRE(cb2.size() == 1);
    REQUIRE(cb0[0] == 1);
    REQUIRE(cb0[1] == 2);
    REQUIRE(cb2[0] == 3);
}

TEST_CASE("Test ClusterBoundaries Remove() function", "[ClusterBoundaries]") {
    std::vector<size_t> clusters = {0, 2, 4};
    ClusterBoundaries cbs(clusters, 6, 2);
    cbs.Add(0, 1);
    cbs.Add(0, 2);
    cbs.Add(2, 3);
    cbs.Remove(0, 1);
    cbs.Defragment(0);
    auto cb0 = cbs.GetBoundary(0);
    auto cb2 = cbs.GetBoundary(2);
    REQUIRE(cb0.size() == 1);
    REQUIRE(cb2.size() == 1);
    REQUIRE(cb0[0] == 1);
    REQUIRE(cb2[0] == 3);
}

TEST_CASE("Test ClusterBoundaries Merge() function", "[ClusterBoundaries]") {
    std::vector<size_t> clusters = {0, 2, 4};
    ClusterBoundaries cbs(clusters, 6, 2);
    cbs.Add(0, 1);
    cbs.Add(0, 2);
    cbs.Add(2, 3);
    cbs.Merge(0, 2);
    auto cb0 = cbs.GetBoundary(0);
    REQUIRE(cb0.size() == 3);
    REQUIRE(cb0[0] == 1);
    REQUIRE(cb0[1] == 2);
    REQUIRE(cb0[2] == 3);
}

TEST_CASE("Test ClusterBoundaries Defragment() function",
          "[ClusterBoundaries]") {
    std::vector<size_t> clusters = {0, 2, 4};
    ClusterBoundaries cbs(clusters, 6, 3);
    cbs.Add(0, 1);
    cbs.Add(0, 2);
    cbs.Add(0, 0);
    cbs.Add(2, 3);
    cbs.Defragment(0);
    auto cb0 = cbs.GetBoundary(0);
    REQUIRE(cb0.size() == 3);
    REQUIRE(cb0[0] == 1);
    REQUIRE(cb0[1] == 2);
    REQUIRE(cb0[2] == 0);
}
