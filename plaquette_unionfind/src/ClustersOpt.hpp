#pragma once

#include <queue>
#include <vector>

#include "ClusterBoundary.hpp"
#include "DecodingGraph.hpp"
#include "LatticeVisualizer.hpp"
#include "StabilizerCode.hpp"
#include "Types.hpp"

namespace Plaquette {

/**
 * @brief Represents a set of clusters used in decoding a quantum
 * error-correcting code.
 *
 */
class ClustersOpt {
private:
  DecodingGraph
      decoding_graph_; ///< The decoding graph used to construct the clusters.

  float max_growth_;

  std::vector<float> edge_growth_; ///< The growth of each edge.
  std::vector<float>
      edge_growth_increment_;       ///< The increment in growth for each edge.
  std::vector<int> cluster_parity_; ///< The parity of each cluster.
  std::vector<bool> fully_grown_edges_; ///< Indicates which edges have
                                        ///< reached maximum growth.

  std::vector<int> vertex_to_cluster_id_;
  std::vector<bool> syndrome_;

public:
  /**
   * @brief Constructs a new `Clusters`.
   *
   * @param decoding_graph The decoding graph used to construct the clusters.
   * @param initial_cluster_roots The initial cluster roots.
   * @param initial_cluster_edges The initial cluster edges.
   * @param edge_growth_increments The increments in growth for each edge.
   * @param max_growth The maximum growth for each edge.
   */
  ClustersOpt(const DecodingGraph &decoding_graph,
              const std::vector<bool> &syndrome = {},
              const std::vector<bool> &initial_cluster_edges = {},
              const std::vector<float> &edge_growth_increment = {},
              float max_growth = 2.0)
      : decoding_graph_(decoding_graph), syndrome_(syndrome) {

    syndrome_ = syndrome;
    max_growth_ = max_growth;

    edge_growth_increment_ =
        edge_growth_increment.empty()
            ? std::vector<float>(decoding_graph.GetNumLocalEdges(), 1.0)
            : edge_growth_increment;

    fully_grown_edges_ =
        initial_cluster_edges.empty()
            ? std::vector<bool>(decoding_graph.GetNumLocalEdges(), false)
            : initial_cluster_edges;

    // num_fully_grown_edges_ =
    // std::vector<size_t>(decoding_graph.GetNumVertices(), 0);
    edge_growth_ = std::vector<float>(decoding_graph.GetNumLocalEdges(), 0.0);
    cluster_parity_ = std::vector<int>(decoding_graph.GetNumVertices(), 0);
    vertex_to_cluster_id_ =
        std::vector<int>(decoding_graph.GetNumVertices(), -1);

    InitClusterRoots_(syndrome);
  }

  auto GetEdgeGrowth() const { return edge_growth_; }

  /**
   * @brief Initialize the roots of the clusters.
   *
   * This method initializes the roots of the clusters by finding all the
   * vertices with non-zero syndrome that are not already in a cluster, and
   * creating a new cluster with each such vertex as its root. It updates the
   * vertex-to-cluster, cluster-to-boundary-vertex, and initial cluster
   * vectors, and initializes the cluster parity to 1 for each new cluster.
   *
   * @param syndrome The syndrome of the code.
   */
  void InitClusterRoots_(const std::vector<bool> &syndrome) {
    for (size_t g = 0; g < syndrome.size(); g++) {
      if (syndrome[g] && vertex_to_cluster_id_[g] == -1) {
        vertex_to_cluster_id_[g] = g;
        cluster_parity_[g] = 1;
      }
    }
  }

  void GrowClusters() {
    for (size_t vertex_id; vertex_id < decoding_graph_.GetNumVertices();
         vertex_id++) {
      size_t stride = decoding_graph_.GetLocalEdgeStride(vertex_id);
      for (size_t local_edge_id = 0;
           local_edge_id <
           decoding_graph_.GetEdgesTouchingVertex(vertex_id).size();
           local_edge_id++) {
        edge_growth_[stride + local_edge_id] +=
            (vertex_to_cluster_id_[vertex_id] != -1 and
             cluster_parity_[vertex_to_cluster_id_[vertex_id]] == 1) *
            edge_growth_increment_[stride + local_edge_id];
      }
    }
  }

  void GrowClusters(const std::vector<bool> &cluster_mask) {
    for (size_t vertex_id; vertex_id < decoding_graph_.GetNumVertices();
         vertex_id++) {
      size_t stride = decoding_graph_.GetLocalEdgeStride(vertex_id);
      for (size_t local_edge_id;
           local_edge_id < decoding_graph_.GetNumLocalEdges();
           local_edge_id++) {
        edge_growth_[stride + local_edge_id] +=
            cluster_mask[vertex_to_cluster_id_[vertex_id]] *
            (cluster_parity_[vertex_to_cluster_id_[vertex_id]] == 1) *
            edge_growth_increment_[stride + local_edge_id];
      }
    }
  }

  inline std::vector<size_t> FindEdgesToFuse() const {
    std::vector<size_t> edges_to_fuse(decoding_graph_.GetNumEdges());
    
    for (size_t edge_id = 0; edge_id < decoding_graph_.GetNumEdges(); edge_id++) {
      std::pair<size_t, size_t> local_edges = edge_id_to_local_edges_[edge_id];
      edges_to_fuse[edge_id] =
          !fully_grown_edges_[edge_id] and
          (edge_growth_[local_edges.first] + edge_growth_[local_edges.second] >=
           max_growth_);
    }
    
    for (size_t edge_id = 0; edge_id < decoding_graph_.GetNumEdges();
         edge_id++) {
      if (edges_to_fuse[edge_id]) {
        std::pair<size_t, size_t> vertices =
            decoding_graph_.GetVerticesTouchingEdge(edge_id);
        int cluster_root = std::max({vertex_to_cluster_id_[vertices.first],
                                     vertex_to_cluster_id_[vertices.second]});
        vertices_to_fuse[vertices.first] =
            vertices_to_fuse[cluster_root] == -1
                ? cluster_root
                : vertices_to_fuse[vertices.first];
        vertices_to_fuse[vertices.second] =
            vertices_to_fuse[cluster_root] == -1
                ? cluster_root
                : vertices_to_fuse[vertices.first];
      }
    }
    return vertices_to_fuse;
  }

  inline void FuseEdges(const std::vector<size_t> &vertices_to_fuse) {
    for (size_t vertex_id = 0; vertex_id < decoding_graph_.GetNumVertices();
         vertex_id++) {
      vertex_to_cluster_id_[vertex_id] = (vertices_to_fuse[vertex_id] != -1)
	? vertices_to_fuse[vertex_id]
	: vertex_to_cluster_id_[vertex_id];
    }
  }

  inline UpdateClusterParity() {
    for (size_t vertex_id = 0; vertex_id < decoding_graph_.GetNumVertices();
         vertex_id++) {

      cluster_parity_[vertex_to_cluster_id_[vertex_id]] =
	(syndrome_[vertex_id] == -1 or
	 cluster_parity_[vertex_to_cluster_id_[vertex_id]] == -1)
	? -1
	: cluster_parity_[vertex_to_cluster_id_[vertex_id]] +
	syndrome_[vertex_id];
      
    }
  }

  // inline std::vector<size_t> GetFuseList(){
  //   std::vector<size_t> fuse_list(num_vertices);
  //   for (size_t i = 0; i < possible_global_edges_to_fuse.size(); i++){
  //     std::vector<size_t,size_t> global_edge_to_vertices =
  //     global_edge_to_vertices_[i]; fuse_list[global_edge_to_vertices.second]
  //     = (possible_global_edges_to_fuse[i] and
  //     vertex_to_root[global_edge_to_vertices.first] != -1) ?
  //     global_edge_to_vertices.first : global_edge_to_vertices.second;
  //     fuse_list[global_edge_to_vertices.first] =
  //     (possible_global_edges_to_fuse[i] and
  //     vertex_to_root[global_edge_to_vertices.first] != -1) ?
  //     global_edge_to_vertices.first : global_edge_to_vertices.second;
  //   }
  //   return fuse_list;
  // }

  // inline std::vector<size_t> FuseEdges(){
  //   for (size_t i = 0; i < fuse_list.size(); i++){
  //     vertex_to_cluster_id_[i] = fuse_list[i]; //maybe don't even need this,
  //     just need cluster_parity_ cluster_parity_[i] = (cluster_parity_[i] ==
  //     -1 or cluster_parity_[y] == -1) ? -1 : cluster_parity_[x] +
  //     cluster_parity_[y];
  //   }\
  // }
};
}; // namespace Plaquette
