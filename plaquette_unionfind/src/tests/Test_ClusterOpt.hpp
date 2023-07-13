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
      syndrome[4] = true;
      syndrome[7] = true;
      ClustersOpt cluster_set(dg, syndrome, {}, {});

      auto cluster_parity = cluster_set.GetClusterParity();
      for (size_t i = 0; i < cluster_parity.size(); i++) {
	std::cout << "parity = " << cluster_parity[i] << std::endl;
      }

      
      cluster_set.GrowClusters();
      auto local_edges = cluster_set.GetEdgeGrowth();
      
      for (size_t i = 0; i < local_edges.size(); i++) {
	auto global_edge = dg.GetGlobalEdgeFromLocalEdge(local_edges[i]);
	auto vertices = dg.GetVerticesConnectedByEdge(global_edge);
	std::cout << i << " " << local_edges[i] << std::endl;
      }

      // LatticeVisualizerDB db;
      // db.AddVisualizer("before_fuse",cluster_set.GetVisualizer(code, StabilizerCode::GridType::Z, true));
      // db.Plot("before_fuse");

      // auto vertices_to_fuse = FindEdgesToFuse();
      auto vertices_to_fuse = cluster_set.FindNewFullyGrownEdges();
      cluster_set.FuseEdges(vertices_to_fuse);
      for (size_t i = 0; i < vertices_to_fuse.size(); i++) {
	std::cout << i << " " << vertices_to_fuse[i] << std::endl;
      }

      auto vertex_to_cluster_id = cluster_set.GetVertexToClusterId();
      for (size_t i = 0; i < vertex_to_cluster_id.size(); i++) {
	std::cout << i << " " << vertex_to_cluster_id[i] << std::endl;
      }

      cluster_set.UpdateClusterParity();
      cluster_parity = cluster_set.GetClusterParity();

      for (size_t i = 0; i < cluster_parity.size(); i++) {
	REQUIE(cluster_parity[i] == 0);
      }
      
      // db.AddVisualizer("after_fuse",cluster_set.GetVisualizer(code, StabilizerCode::GridType::Z, true));
      // db.Plot("after_fuse");
      
    }
    
}
