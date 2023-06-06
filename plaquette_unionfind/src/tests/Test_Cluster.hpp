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

TEST_CASE("GrowCluster returns the correct possible edges to fuse") {
    // Set up a decoding graph with 6 vertices and 5 edges.
    Plaquette::DecodingGraph graph(
        6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
        {true, false, true, false, false, false});

    // Set up a Clusters with initial clusters {1} and {4}.
    std::vector<bool> syndrome = {false, true, false, false, true, false};
    std::vector<bool> initial_cluster_edges = {false, false, false,
                                               false, false, true};

    std::vector<float> edge_increments = {1, 1.5, 1, 1, 1, 1};
    Clusters cluster_set(graph, syndrome, initial_cluster_edges,
                         edge_increments);

    SECTION("Possible edges to fuse are correctly identified") {

        REQUIRE(cluster_set.GetInitialClusters().size() == 3);
        REQUIRE(cluster_set.GetInitialClusters()[0] == 3);
        REQUIRE(cluster_set.GetInitialClusters()[1] == 1);
        REQUIRE(cluster_set.GetInitialClusters()[2] == 4);
        REQUIRE(cluster_set.GetFullyGrownEdges()[5] == true);
        REQUIRE(cluster_set.GetFullyGrownEdges()[0] == false);
        REQUIRE(cluster_set.GetFullyGrownEdges()[1] == false);
        REQUIRE(cluster_set.GetFullyGrownEdges()[2] == false);
        REQUIRE(cluster_set.GetFullyGrownEdges()[3] == false);
        REQUIRE(cluster_set.GetFullyGrownEdges()[4] == false);
        REQUIRE(cluster_set.GetClusterGrowth()[3] ==
                cluster_set.GetMaxGrowth());

        std::vector<size_t> possible_edges = cluster_set.GrowCluster(1);
        REQUIRE(possible_edges.size() == 0);
        possible_edges = cluster_set.GrowCluster(4);
        REQUIRE(possible_edges.size() == 1);
        REQUIRE(possible_edges[0] == 4);
    }

    SECTION("Cluster growth is correctly updated") {
        // Grow the cluster.
        cluster_set.GrowCluster(1);

        // Check that the cluster growth has been updated correctly.
        REQUIRE(cluster_set.GetClusterGrowth()[1] == 1.5 + 1 + 1);
    }

    SECTION("Boundary vertices are correctly identified") {
        // Grow the cluster.
        cluster_set.GrowCluster(1);
        cluster_set.GrowCluster(1);

        REQUIRE(cluster_set.GetPhysicalBoundaryVertices()[0]);
        REQUIRE(cluster_set.GetPhysicalBoundaryVertices()[2]);
        REQUIRE(cluster_set.GetFullyGrownEdges()[0] == true);
        REQUIRE(cluster_set.GetFullyGrownEdges()[1] == true);
        REQUIRE(cluster_set.GetFullyGrownEdges()[2] == false);
        REQUIRE(cluster_set.GetFullyGrownEdges()[3] == false);
        REQUIRE(cluster_set.GetFullyGrownEdges()[4] == true);
        REQUIRE(cluster_set.GetFullyGrownEdges()[5] == true);
        REQUIRE(cluster_set.GetPhysicalBoundaryVertices()[2]);
        REQUIRE(cluster_set.GetNumPhysicalBoundaryVertices() == 2);
        REQUIRE(cluster_set.GetClusterParity()[1] == -1);
    }

    SECTION("New vertices are correctly assigned to the cluster") {
        // Grow the cluster
        cluster_set.GrowCluster(1);
        cluster_set.GrowCluster(1);

        // Vertex 0 and 2 should now be assigned to cluster 1.
        REQUIRE(cluster_set.GetVertexToClusterId()[0] == 1);
        REQUIRE(cluster_set.GetVertexToClusterId()[2] == 1);
    }

    SECTION("Boundary vertices are correctly added to the cluster") {
        // Grow the cluster.
        cluster_set.GrowCluster(1);
        cluster_set.GrowCluster(1);

        auto &&boundary_vertices =
            cluster_set.GetClusterBoundary().GetBoundary(1);

        REQUIRE(boundary_vertices.size() == 3);
        REQUIRE(boundary_vertices.contains(0) == true);
        REQUIRE(boundary_vertices.contains(1) == true);
        REQUIRE(boundary_vertices.contains(2) == true);
    }
}

TEST_CASE("MergeClusters correctly merges two clusters") {
    // Set up a decoding graph with 6 vertices and 5 edges.
    Plaquette::DecodingGraph graph(
        6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
        {true, false, true, false, false, false});

    // Set up a Clusters with initial clusters {0} and {2}.
    std::vector<bool> syndrome = {false, true, false, false, true, false};
    std::vector<bool> initial_cluster_edges = {false, false, false,
                                               false, false, true};
    std::vector<float> edge_increments = {1, 1.5, 1, 1, 1, 1};
    Clusters cluster_set(graph, syndrome, initial_cluster_edges,
                         edge_increments);

    // Merge clusters 0 and 2.
    cluster_set.GrowCluster(1);
    cluster_set.GrowCluster(1);
    cluster_set.MergeClusters(1, 4);

    SECTION("Vertices are correctly assigned to the merged cluster") {
        REQUIRE(cluster_set.GetVertexToClusterId()[0] == 1);
        REQUIRE(cluster_set.GetVertexToClusterId()[2] == 1);
        REQUIRE(cluster_set.GetVertexToClusterId()[1] == 1);
        REQUIRE(cluster_set.GetVertexToClusterId()[4] == 1);
    }

    SECTION("Cluster growth is correctly updated") {
        REQUIRE(cluster_set.GetClusterGrowth()[1] == 1.5 * 2 + 2 + 2);
    }

    SECTION("Cluster parity is correctly updated") {
        REQUIRE(cluster_set.GetClusterParity()[1] == -1);
    }

    SECTION("Boundary vertices are correctly merged") {
        // Check that cluster 0 has boundary vertices {0, 1, 2, 4}.
        auto &&boundary_vertices =
            cluster_set.GetClusterBoundary().GetBoundary(1);
        REQUIRE(boundary_vertices.size() == 4);
        REQUIRE(boundary_vertices.contains(0) == 1);
        REQUIRE(boundary_vertices.contains(1) == 1);
        REQUIRE(boundary_vertices.contains(2) == 1);
        REQUIRE(boundary_vertices.contains(4) == 1);
    }
}

TEST_CASE("CheckBoundaryVertices removes fully grown boundary vertices") {
    // Set up a decoding graph with 6 vertices and 5 edges.
    Plaquette::DecodingGraph graph(
        6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
        {true, false, true, false, false, false});

    // Set up a Clusters with initial clusters {0} and {2}.
    std::vector<bool> syndrome = {false, true, false, false, true, false};
    std::vector<bool> initial_cluster_edges = {false, false, false,
                                               false, false, true};
    std::vector<float> edge_increments = {1, 1.5, 1, 1, 1, 1};
    Clusters cluster_set(graph, syndrome, initial_cluster_edges,
                         edge_increments);

    // Grow the cluster.
    cluster_set.GrowCluster(1);
    cluster_set.GrowCluster(1);

    // Check that the boundary vertices are correctly identified.
    auto &&boundary_vertices = cluster_set.GetClusterBoundary().GetBoundary(1);
    REQUIRE(boundary_vertices.size() == 3);
    REQUIRE(boundary_vertices.contains(0) == 1);
    REQUIRE(boundary_vertices.contains(2) == 1);
    REQUIRE(boundary_vertices.contains(1) == 1);
    REQUIRE(boundary_vertices.contains(4) == 0);
    REQUIRE(boundary_vertices.contains(5) == 0);
    REQUIRE(boundary_vertices.contains(3) == 0);

    // CheckBoundaryVertices should remove vertex 1 and vertex 4 from the
    // boundary vertices.
    cluster_set.MergeClusters(1, 4);
    cluster_set.CheckBoundaryVertices(1);

    auto &&boundary_vertices_2 =
        cluster_set.GetClusterBoundary().GetBoundary(1);
    REQUIRE(boundary_vertices_2.size() == 1);
    REQUIRE(boundary_vertices_2.contains(0) == 0);
    REQUIRE(boundary_vertices_2.contains(2) == 0);
    REQUIRE(boundary_vertices_2.contains(1) == 0);
    REQUIRE(boundary_vertices_2.contains(4) == 1);
    REQUIRE(boundary_vertices_2.contains(5) == 0);
    REQUIRE(boundary_vertices_2.contains(3) == 0);
}

TEST_CASE("FindClusterRoot finds the correct cluster root") {
    // Set up a decoding graph with 6 vertices and 5 edges.
    Plaquette::DecodingGraph graph(
        6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
        {true, false, true, false, false, false});

    // Set up a Clusters with initial clusters {0} and {2}.
    std::vector<bool> syndrome = {false, true, false, false, true, false};
    std::vector<bool> initial_cluster_edges = {false, false, false,
                                               false, false, true};
    std::vector<float> edge_increments = {1, 1.5, 1, 1, 1, 1};
    Clusters cluster_set(graph, syndrome, initial_cluster_edges,
                         edge_increments);

    SECTION("Root of a vertex in the merged cluster is the root of the merged "
            "cluster") {
        // The root of vertex 1 (which was in cluster 1 before the merge) should
        // be the root of the merged cluster.

        REQUIRE(cluster_set.FindClusterRoot(0) == -1);
        REQUIRE(cluster_set.FindClusterRoot(2) == -1);
        REQUIRE(cluster_set.FindClusterRoot(4) == 4);
        REQUIRE(cluster_set.FindClusterRoot(1) == 1);
        REQUIRE(cluster_set.FindClusterRoot(5) == 3);
        REQUIRE(cluster_set.FindClusterRoot(3) == 3);
    }

    SECTION("Root of a vertex in the merged cluster is the root of the merged "
            "cluster") {
        // Grow the cluster.
        cluster_set.GrowCluster(1);
        cluster_set.GrowCluster(1);

        REQUIRE(cluster_set.FindClusterRoot(0) == 1);
        REQUIRE(cluster_set.FindClusterRoot(1) == 1);
        REQUIRE(cluster_set.FindClusterRoot(2) == 1);
    }
}

TEST_CASE("GetSmallestClusterWithOddParity returns the correct cluster ID") {
    // Set up a decoding graph with 6 vertices and 5 edges.
    Plaquette::DecodingGraph graph(
        6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
        {true, false, true, false, false, false});

    // Set up a Clusters with initial clusters {0} and {2}.
    std::vector<bool> syndrome = {false, true, false, false, true, false};
    std::vector<bool> initial_cluster_edges = {false, false, false,
                                               false, false, true};
    std::vector<float> edge_increments = {1, 1.5, 1, 1, 1, 1};
    Clusters cluster_set(graph, syndrome, initial_cluster_edges,
                         edge_increments);

    REQUIRE((cluster_set.GetSmallestClusterWithOddParity() == 1 ||
             cluster_set.GetSmallestClusterWithOddParity() == 4));

    // Grow the cluster.
    cluster_set.GrowCluster(1);
    cluster_set.GrowCluster(1);

    REQUIRE(cluster_set.GetSmallestClusterWithOddParity() == 4);
}
