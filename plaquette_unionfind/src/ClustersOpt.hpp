#pragma once

#include <queue>
#include <vector>

#include "ClusterBoundary.hpp"
#include "DecodingGraph.hpp"
#include "LatticeVisualizer.hpp"
#include "StabilizerCode.hpp"
#include "Types.hpp"

namespace Plaquette {

  struct Sizes {
    std::vector<size_t> boundary_sizes; // size of the cluster boundary (in # of vertices)
    std::vector<float> cluster_sizes; // size of the cluster (sum over all edge lengths)
    std::vector<size_t> erasure_sizes; // size of the erasure in each cluster (sum over all fully grown edges);
  };

  enum class CompareSizeMethod {
    BOUNDARY_SIZE_THEN_CLUSTER_SIZE,
    BOUNDARY_SIZE_THEN_ERASURE_SIZE
  };
  
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
  Sizes sizes_;
  
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

    // for (size_t i = 0; i < cluster_parity_.size(); i++){
    //   cluster_parity_[i] = decoding_graph.IsVertexOnBoundary(i) ? -1 : 0;
    // }
    
    InitClusterRoots_(syndrome);
  }

  auto GetEdgeGrowth() const { return edge_growth_; }
  auto GetVertexToClusterId() const { return vertex_to_cluster_id_; }
  auto GetClusterParity() const { return cluster_parity_; }

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

  std::vector<int> FindNewFullyGrownEdges() {
    std::vector<size_t> edges_to_fuse(decoding_graph_.GetNumEdges());
    std::vector<int> vertices_to_fuse(decoding_graph_.GetNumVertices(),-1);
    
    for (size_t edge_id = 0; edge_id < decoding_graph_.GetNumEdges(); edge_id++) {
      auto local_edge_left = decoding_graph_.GetLocalEdgeFromGlobalEdge(edge_id,0);
      auto local_edge_right = decoding_graph_.GetLocalEdgeFromGlobalEdge(edge_id,1);
      edges_to_fuse[edge_id] =
          !fully_grown_edges_[edge_id] and
          (edge_growth_[local_edge_left] + edge_growth_[local_edge_right] >=
           max_growth_);
      fully_grown_edges_[edge_id] = edges_to_fuse[edge_id] or fully_grown_edges_[edge_id];
    }
    
    for (size_t edge_id = 0; edge_id < decoding_graph_.GetNumEdges();
         edge_id++) {
      if (edges_to_fuse[edge_id]) {
        std::pair<size_t, size_t> vertices =
            decoding_graph_.GetVerticesConnectedByEdge(edge_id);
        int cluster_root = std::max({vertex_to_cluster_id_[vertices.first],
                                     vertex_to_cluster_id_[vertices.second]});
        vertices_to_fuse[vertices.first] =
            vertices_to_fuse[cluster_root] == -1
                ? cluster_root
                : vertices_to_fuse[cluster_root];
	
        vertices_to_fuse[vertices.second] =
            vertices_to_fuse[cluster_root] == -1
                ? cluster_root
                : vertices_to_fuse[cluster_root];
      }
    }
    return vertices_to_fuse;
  }

  inline void FuseEdges(const std::vector<int> &vertices_to_fuse) {
    for (size_t vertex_id = 0; vertex_id < decoding_graph_.GetNumVertices();
         vertex_id++) {
      if (vertices_to_fuse[vertex_id] != -1) {
	vertex_to_cluster_id_[vertex_id] = vertices_to_fuse[vertex_id];
      }
      else {
	if (vertices_to_fuse[vertex_to_cluster_id_[vertex_id]] != -1) {
	  vertex_to_cluster_id_[vertex_id] = vertices_to_fuse[vertex_to_cluster_id_[vertex_id]];
	}
      }
    }
  }

  inline int AddParity(int parity_a, int parity_b) {
    return (parity_a < 0 or parity_b < 0) ? -1 : (parity_a + parity_b) == 2 ? 0 : (parity_a + parity_b);
  }

  inline bool UpdateClusterParity() {
    size_t sum = 0;
    for (size_t vertex_id = 0; vertex_id < decoding_graph_.GetNumVertices();
         vertex_id++) {
      cluster_parity_[vertex_to_cluster_id_[vertex_id]] = decoding_graph_.IsVertexOnBoundary(vertex_id) ? -1 : AddParity(cluster_parity_[vertex_to_cluster_id_[vertex_id]],syndrome_[vertex_id]);
      cluster_parity_[vertex_id] = (vertex_id == vertex_to_cluster_id_[vertex_id]) ? cluster_parity_[vertex_id] : 0;
      sum += (cluster_parity_[vertex_id] == 1);
    }
    return sum != 0;
  }


  template <typename Code>
  auto GetVisualizer(Code &code, const StabilizerCode::GridType &grid_type,
		     bool annotate = false) const {

    auto lv = code.GetVisualizer(grid_type, annotate);
    const auto &coords = grid_type == StabilizerCode::GridType::Z
      ? code.GetZStabilizerCoords()
      : code.GetXStabilizerCoords();

    size_t k = 0;
    for (size_t cc = 0; cc < decoding_graph_.GetNumVertices(); cc++) {

      if (vertex_to_cluster_id_[cc] != cc)
	continue;

      size_t c = cc;

      std::string cluster_color = GetHexColor(k);
      VertexPrintProps vpp;
      vpp.vertex = coords[c];
      vpp.marker = "x";
      vpp.annotation = "";
      vpp.color = cluster_color;
      vpp.markersize = 20;
      vpp.fillstyle = "full";
      vpp.label = std::to_string(c) + "_root";
      lv.AddVertexProps(vpp);

      for (size_t v = 0; v < decoding_graph_.GetNumVertices(); ++v) {

	if (vertex_to_cluster_id_[v] == c) {
	  size_t v_stride = decoding_graph_.GetLocalEdgeStride(v);

	  if (vertex_to_cluster_id_[v] == c and v != c) {
	    VertexPrintProps vpp;
	    vpp.vertex = coords[v];
	    vpp.marker = "o";
	    vpp.annotation = "";
	    vpp.color = cluster_color;
	    vpp.markersize = 10;
	    vpp.fillstyle = "full";
	    vpp.label = std::to_string(c) + "_vertex";
	    lv.AddVertexProps(vpp);
	  }

	  const auto &edges =
	    decoding_graph_.GetEdgesTouchingVertex(v);
	  const auto &vertices =
	    decoding_graph_.GetVerticesTouchingVertex(v);

	  for (size_t e = 0; e < edges.size(); ++e) {

	    float edge_growth = edge_growth_[v_stride + e];
	    if (edge_growth > 0) {
	      EdgePrintProps epp;
	      epp.vertex_0 = coords[v];
	      epp.vertex_1 = coords[vertices[e]];
	      if (code.IsPeriodic()) {
		code.FixEdgeCoordsForVisual(epp.vertex_0,
					    epp.vertex_1);
	      }
	      epp.linestyle = "-";
	      epp.linewidth = 20;
	      epp.color = cluster_color;
	      epp.label = std::to_string(c) + "_edge";
	      epp.alpha = 1.0;
	      epp.fraction = (edge_growth > 2.0) ? 1.0 : edge_growth / 2.0;
	      epp.annotation = "";
	      lv.AddEdgeProps(epp);
	    }
	  }
	}
      }
      k++;
    }
    return lv;
  };


  
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
