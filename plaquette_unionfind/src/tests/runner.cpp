#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "Test_Cluster.hpp"
#include "Test_ClusterBoundary.hpp"
#include "Test_StabilizerCode.hpp"
#include "Test_UnionFind.hpp"

int main(int argc, char *argv[]) {
    int result;
    result = Catch::Session().run(argc, argv);
    return result;
}
