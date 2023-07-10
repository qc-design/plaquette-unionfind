#include "ClustersOpt.hpp"
#include "DecodingGraph.hpp"
#include "Utils.hpp"
#include <catch2/catch.hpp>

using namespace Plaquette;

TEST_CASE("ClustersOpt GrowCluster returns the correct possible edges to fuse") {
    // Set up a decoding graph with 6 vertices and 5 edges.
    Plaquette::DecodingGraph graph(
        6, {{0, 1}, {1, 2}, {3, 4}, {4, 5}, {1, 4}, {3, 5}},
        {true, false, true, false, false, false});

    std::vector<bool> syndrome = {false, true, false, false, true, false};
    std::vector<float> edge_increments(graph.GetNumLocalEdges(), 1);
    ClustersOpt cluster_set(graph, syndrome, {},
			    {});

    SECTION("Possible edges to fuse are correctly identified") {

      cluster_set.GrowClusters();
      cluster_set.GetEdgeGrowth();
      
      for (auto edge : cluster_set.GetEdgeGrowth()) {
	std::cout << edge << std::endl;
      }
	
    }

    SECTION("Find edges to fuse") {
      
      ToricCode code(3);
      auto dg = code.GetZStabilizerDecodingGraph();
      std::vector<bool> syndrome(dg.GetNumVertices(), false);
      syndrome[1] = true;
      syndrome[5] = true;
      syndrome[7] = true;
      ClustersOpt cluster_set(dg, syndrome, {},
			    {});

      cluster_set.GrowClusters();
      auto local_edges = cluster_set.GetEdgeGrowth();
      for (size_t i = 0; i < local_edges.size(); i++) {
	std::cout << local_edges[i] << std::endl;
      }
      // auto vertices_to_fuse = FindEdgesToFuse();
      
    }
    
}
