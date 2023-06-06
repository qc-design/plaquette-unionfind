#pragma once

#include "Clusters.hpp"
#include "DecodingGraph.hpp"
#include "PeelingDecoder.hpp"

namespace Plaquette {
namespace Decoders {

/**
 * @brief Decoder based on the union-find algorithm.
 *
 * This decoder uses the union-find algorithm to grow clusters of connected
 * vertices in the decoding graph, and then applies peeling decoding to
 * correct errors in the code. The decoder can handle both erasure and
 * weight-1 errors, and supports the use of weights and max-growth parameters
 * to control the cluster growth.
 */
class UnionFindDecoder {

  private:
    Clusters cluster_set_;         /**< The union-find cluster set. */
    DecodingGraph decoding_graph_; /**< The decoding graph. */

  public:
    /**
     * @brief Constructor for the union-find decoder.
     *
     * @param decoding_graph The decoding graph to use.
     * @param syndrome The syndrome of the code.
     * @param erasure The erasure pattern of the code.
     * @param weights (optional) The weights of the edges in the decoding graph.
     * @param max_growth (optional) The maximum growth factor for the clusters.
     */
    UnionFindDecoder(const DecodingGraph &decoding_graph,
                     const std::vector<float> &edge_increments = {},
                     float max_growth = 2.0)
        : cluster_set_(decoding_graph, {}, {}, edge_increments, max_growth),
          decoding_graph_(decoding_graph) {}

    /**
     * @brief Get the union-find cluster set.
     *
     * @return The union-find cluster set.
     */
    const auto &GetClusterSet() const { return cluster_set_; }
    auto &GetClusterSet() { return cluster_set_; }

    /**
     * @brief Get the modified erasure pattern.
     *
     * @return The modified erasure pattern.
     */
    const auto &GetModifiedErasure() {
        return cluster_set_.GetFullyGrownEdges();
    }

    /**
     * @brief Perform one iteration of syndrome validation for a given cluster.
     *
     * This method grows the given cluster by adding new edges to it, and then
     * merges clusters that are connected by these edges. It also checks the
     * boundary vertices of any new clusters that are formed.
     *
     * @param cluster_id The ID of the cluster to validate.
     */
    void SyndromeValidationIteration(size_t cluster_id) {
        const auto &&edges_to_fuse = cluster_set_.GrowCluster(cluster_id);
        std::unordered_set<size_t> new_roots = {cluster_id};
        for (const auto &edge_id : edges_to_fuse) {
            auto &vertices =
                decoding_graph_.GetVerticesConnectedByEdge(edge_id);
            auto &u = vertices.first;
            auto &v = vertices.second;
            auto &&u_root = cluster_set_.FindClusterRoot(u);
            auto &&v_root = cluster_set_.FindClusterRoot(v);
            if (u_root != v_root) {
                new_roots.insert(cluster_set_.MergeClusters(u_root, v_root));
            }
        }
        for (const auto &root : new_roots) {
            cluster_set_.CheckBoundaryVertices(root);
            cluster_set_.AddToGrowQueue(root);
        }
    }

    /**
     * @brief Perform syndrome validation on all clusters.
     *
     * This method repeatedly calls SyndromeValidationIteration() for the
     * smallest cluster with odd parity until there are no such clusters left.
     */
    void SyndromeValidation() {
        int cluster_id = cluster_set_.GetSmallestClusterWithOddParity();
        while (cluster_id != -1) {
            SyndromeValidationIteration(cluster_id);
            cluster_id = cluster_set_.GetSmallestClusterWithOddParity();
        }
    }

    inline void SetSyndromeAndErasure(const std::vector<bool> &syndrome,
                                      const std::vector<bool> &erasure) {
        cluster_set_.InitEdgesRecursive_(erasure, syndrome);
        cluster_set_.InitClusterRoots_(syndrome);
    }

    inline void SetSyndrome(const std::vector<bool> &syndrome) {
        cluster_set_.InitClusterRoots_(syndrome);
    }

    std::vector<bool> Decode(std::vector<bool> &syndrome) {
        SetSyndrome(syndrome);
        SyndromeValidation();
        return PeelingDecoder().Decode(
            decoding_graph_, syndrome, cluster_set_.GetFullyGrownEdges(),
            cluster_set_.GetPhysicalBoundaryVertices(),
            cluster_set_.GetNumPhysicalBoundaryVertices());
    }

    std::vector<bool> Decode(std::vector<bool> &syndrome,
                             const std::vector<bool> &erasure) {
        SetSyndromeAndErasure(syndrome, erasure);
        SyndromeValidation();
        return PeelingDecoder().Decode(
            decoding_graph_, syndrome, cluster_set_.GetFullyGrownEdges(),
            cluster_set_.GetPhysicalBoundaryVertices(),
            cluster_set_.GetNumPhysicalBoundaryVertices());
    }
};
}; // namespace Decoders
}; // namespace Plaquette
