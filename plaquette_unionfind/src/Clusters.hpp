#pragma once

#include <queue>
#include <vector>

#include "ClusterBoundary.hpp"
#include "DecodingGraph.hpp"
#include "LatticeVisualizer.hpp"
#include "StabilizerCode.hpp"
#include "Types.hpp"

namespace Plaquette {

template <typename... Types> struct Compare {
    template <size_t Index>
    bool compareElements(const std::tuple<Types...> &a,
                         const std::tuple<Types...> &b) {
        if constexpr (Index < sizeof...(Types)) {
            if (std::get<Index>(a) == std::get<Index>(b)) {
                return compareElements<Index + 1>(a, b);
            } else {
                return std::get<Index>(a) > std::get<Index>(b);
            }
        } else {
            return false;
        }
    }

    bool operator()(const std::tuple<Types...> &a,
                    const std::tuple<Types...> &b) {
        return compareElements<0>(a, b);
    }
};

template <typename... Types>
using PriorityQueue =
    std::priority_queue<std::tuple<Types...>, std::vector<std::tuple<Types...>>,
                        Compare<Types...>>;

/**
 * @brief Represents a set of clusters used in decoding a quantum
 * error-correcting code.
 *
 */
class Clusters {
  private:
    DecodingGraph
        decoding_graph_; ///< The decoding graph used to construct the clusters.

    float max_growth_;
    std::vector<int>
        vertex_to_cluster_id_;       ///< Mapping from vertex ID to cluster ID.
    std::vector<float> edge_growth_; ///< The growth of each edge.
    std::vector<float>
        edge_growth_increment_; ///< The increment in growth for each edge.
    std::vector<int> cluster_parity_;     ///< The parity of each cluster.
    std::vector<bool> fully_grown_edges_; ///< Indicates which edges have
                                          ///< reached maximum growth.

    std::vector<float>
        cluster_growth_; ///< The growth (sum of edge lengths) of each cluster.
    std::vector<bool> syndrome_;

    std::vector<size_t> initial_clusters_;
    // std::vector<size_t> num_fully_grown_edges_;
    std::vector<bool> physical_boundary_vertices_;
    size_t num_physical_boundary_vertices_;

    ClusterBoundaries cluster_boundary_;
    PriorityQueue<size_t, float, size_t> grow_queue_;

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
    Clusters(const DecodingGraph &decoding_graph,
             const std::vector<bool> &syndrome = {},
             const std::vector<bool> &initial_cluster_edges = {},
             const std::vector<float> &edge_growth_increment = {},
             float max_growth = 2.0)
        : decoding_graph_(decoding_graph), syndrome_(syndrome) {

        syndrome_ = syndrome;
        max_growth_ = max_growth;
        cluster_growth_ =
            std::vector<float>(decoding_graph.GetNumVertices(), 0.0);

        edge_growth_increment_ =
            edge_growth_increment.empty()
                ? std::vector<float>(decoding_graph.GetNumEdges(), 1.0)
                : edge_growth_increment;

        fully_grown_edges_ =
            initial_cluster_edges.empty()
                ? std::vector<bool>(decoding_graph.GetNumEdges(), false)
                : initial_cluster_edges;

        physical_boundary_vertices_ =
            std::vector<bool>(decoding_graph.GetNumVertices(), false);

        // num_fully_grown_edges_ =
        // std::vector<size_t>(decoding_graph.GetNumVertices(), 0);
        edge_growth_ = std::vector<float>(decoding_graph.GetNumEdges(), 0.0);
        cluster_parity_ = std::vector<int>(decoding_graph.GetNumVertices(), 0);
        vertex_to_cluster_id_ =
            std::vector<int>(decoding_graph.GetNumVertices(), -1);

        num_physical_boundary_vertices_ = 0;

        size_t num_vertices = decoding_graph.GetNumVertices();
        size_t max_boundary_size = num_vertices * 6;

        cluster_boundary_ = ClusterBoundaries(num_vertices, max_boundary_size);
        InitEdgesRecursive_(initial_cluster_edges, syndrome);
        InitClusterRoots_(syndrome);
    }

    auto &GetClusterBoundary() { return cluster_boundary_; }

    const auto &GetDecodingGraph() const { return decoding_graph_; }
    std::vector<bool> &GetSyndrome() { return syndrome_; }

    const auto &GetEdgeGrowth() const { return edge_growth_; }

    const auto &GetClusterGrowth() const { return cluster_growth_; }

    const auto &GetMaxGrowth() const { return max_growth_; }

    const auto &GetEdgeGrowthIncrement() const {
        return edge_growth_increment_;
    }

    const auto &GetPhysicalBoundaryVertices() const {
        return physical_boundary_vertices_;
    }

    auto GetNumPhysicalBoundaryVertices() const {
        return num_physical_boundary_vertices_;
    }

    const auto &GetVertexToClusterId() const { return vertex_to_cluster_id_; }

    const auto &GetClusterParity() const { return cluster_parity_; }

    const auto &GetInitialClusters() const { return initial_clusters_; }

    /**
     * @brief Returns a vector indicating which edges have reached maximum
     * growth.
     *
     * @return The vector of fully grown edges.
     */
    const std::vector<bool> &GetFullyGrownEdges() const {
        return fully_grown_edges_;
    }

    /**
     * @brief Returns the set of clusters.
     *
     * @return The set of clusters.
     */
    const auto &GetClusters() const { return initial_clusters_; }

    /**
     * @brief Determines if a vertex has fully grown edges.
     *
     * @param vertex_id The ID of the vertex to check.
     * @return `true` if the vertex has not fully grown edges, `false`
     * otherwise.
     */
    bool IsVertexNotFullyGrown(size_t vertex_id) const {
        const auto &edges = decoding_graph_.GetEdgesTouchingVertex(vertex_id);
        size_t fully_grown = 0;
        for (size_t edge_id = 0; edge_id < edges.size(); edge_id++) {
            fully_grown += fully_grown_edges_[edges[edge_id]];
        }
        return fully_grown < edges.size();
    }
    /**
     * @brief Add an edge to a cluster in the cluster set.
     *
     * This method adds the given edge to the cluster with the given ID in the
     * cluster set. It updates the vertex-to-cluster and
     * cluster-to-boundary-vertex mappings, and updates the parity of the
     * cluster. If either of the vertices connected by the edge is on the
     * physical boundary of the code, it sets the corresponding flag and updates
     * the cluster parity to -1.
     *
     * @param cluster_id The ID of the cluster to add the edge to.
     * @param edge_id The ID of the edge to add.
     * @param syndrome The syndrome of the code.
     */
    void AddEdgeToCluster_(size_t cluster_id, size_t edge_id,
                           const std::vector<bool> &syndrome,
                           std::vector<bool> &syndrome_visited) {
        const auto &vertices =
            decoding_graph_.GetVerticesConnectedByEdge(edge_id);
        vertex_to_cluster_id_[vertices.first] = cluster_id;
        vertex_to_cluster_id_[vertices.second] = cluster_id;

        cluster_parity_[cluster_id] +=
            !syndrome_visited[vertices.first] * syndrome[vertices.first];
        cluster_parity_[cluster_id] +=
            !syndrome_visited[vertices.second] * syndrome[vertices.second];

        syndrome_visited[vertices.first] = true;
        syndrome_visited[vertices.second] = true;

        edge_growth_[edge_id] = max_growth_;
        fully_grown_edges_[edge_id] = true;
        cluster_growth_[cluster_id] += max_growth_;
        // num_fully_grown_edges_[cluster_id] += 1;

        if (IsVertexNotFullyGrown(vertices.first)) {
            cluster_boundary_.Add(cluster_id, vertices.first);
        }
        if (IsVertexNotFullyGrown(vertices.second)) {
            cluster_boundary_.Add(cluster_id, vertices.second);
        }
        if (decoding_graph_.IsVertexOnBoundary(vertices.first)) {
            physical_boundary_vertices_[vertices.first] = true;
            num_physical_boundary_vertices_++;
            cluster_parity_[cluster_id] = -1;
        }
        if (decoding_graph_.IsVertexOnBoundary(vertices.second)) {
            physical_boundary_vertices_[vertices.second] = true;
            num_physical_boundary_vertices_++;
            cluster_parity_[cluster_id] = -1;
        }
    }

    /**
     * @brief Initializes the clusters and their edges using a recursive
     * depth-first search approach.
     *
     * This function initializes the clusters and their edges by traversing the
     * graph recursively in a depth-first search approach. It starts from each
     * edge that belongs to an initial cluster and marks all edges and vertices
     * that belong to the same cluster. The function updates the internal state
     * of the Clusters object by calling the function
     * AddEdgeToCluster_() for each edge found to belong to a cluster.
     *
     * @param initial_edges The initial set of edges.
     */
    void InitEdgesRecursive_(const std::vector<bool> &initial_edges,
                             const std::vector<bool> &syndrome) {
        if (initial_edges.empty()) {
            return;
        }
        std::vector<bool> syndrome_visited(syndrome.size(), false);
        std::vector<bool> edges_visited(decoding_graph_.GetNumEdges(), false);
        for (size_t edge_id = 0; edge_id < decoding_graph_.GetNumEdges();
             edge_id++) {
            if (initial_edges[edge_id] and !edges_visited[edge_id]) {
                const auto &vertices =
                    decoding_graph_.GetVerticesConnectedByEdge(edge_id);
                size_t cluster_id = vertices.first;
                initial_clusters_.emplace_back(cluster_id);
                cluster_boundary_.AddCluster(cluster_id);
                InitEdgesRecursiveDFS_(initial_edges, syndrome, edges_visited,
                                       edge_id, cluster_id, syndrome_visited);
                AddToGrowQueue(cluster_id);
            }
        }
    }

    /**
     * @brief Initialize the edges of a cluster recursively using depth-first
     * search.
     *
     * This method initializes the edges of a cluster recursively using
     * depth-first search. It starts at the given edge and adds all the
     * connected edges that are in the initial edge set to the cluster. It
     * updates the vertex-to-cluster and cluster-to-boundary-vertex mappings,
     * and updates the parity of the cluster. If either of the vertices
     * connected by an added edge is on the physical boundary of the code, it
     * sets the corresponding flag and updates the cluster parity to -1.
     *
     * @param initial_edges The initial edge set.
     * @param syndrome The syndrome of the code.
     * @param edges_visited A boolean vector indicating which edges have already
     *                      been visited.
     * @param edge_id The ID of the current edge.
     * @param cluster_id The ID of the current cluster.
     */
    void InitEdgesRecursiveDFS_(const std::vector<bool> &initial_edges,
                                const std::vector<bool> &syndrome,
                                std::vector<bool> &edges_visited,
                                size_t edge_id, size_t cluster_id,
                                std::vector<bool> &syndrome_visited) {
        edges_visited[edge_id] = true;
        AddEdgeToCluster_(cluster_id, edge_id, syndrome, syndrome_visited);

        // get the vertices connected to this edge
        const auto &neighbour_edges =
            decoding_graph_.GetEdgesTouchingEdge(edge_id);

        // add all the edges connected to these vertices
        for (size_t le = 0; le < neighbour_edges.size(); le++) {
            size_t neighbour_edge = neighbour_edges[le];
            if (initial_edges[neighbour_edge] &&
                !edges_visited[neighbour_edge]) {
                InitEdgesRecursiveDFS_(initial_edges, syndrome, edges_visited,
                                       neighbour_edge, cluster_id,
                                       syndrome_visited);
            }
        }
    }
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
                cluster_boundary_.AddCluster(g);
                cluster_boundary_.Add(g, g);
                initial_clusters_.emplace_back(g);
                AddToGrowQueue(g);
            }
        }
    }

    /**
     * @brief Grow a cluster by fusing edges that are fully grown.
     *
     * This method grows the given cluster by fusing edges that are fully grown.
     * It increments the edge and cluster growth values for each edge that is
     * considered for fusion, and adds any vertices that become fully grown to
     * the cluster. If a fully grown edge connects two vertices that are not yet
     * in a cluster, it adds the vertex to the cluster and updates the
     * vertex-to-cluster, cluster-to-boundary-vertex, and cluster parity
     * mappings. If the fully grown edge connects two vertices that are already
     * in different clusters, it returns the IDs of the possible edges to fuse.
     * If a vertex on the physical boundary of the code becomes fully grown, it
     * sets the corresponding flag and updates the cluster parity to -1.
     *
     * @param cluster_id The ID of the cluster to grow.
     *
     * @return A vector of IDs of possible edges to fuse.
     */
    std::vector<size_t> GrowCluster(size_t cluster_id) {
        std::vector<size_t> possible_edges_to_fuse;
        auto &&cbv = cluster_boundary_.GetBoundary(cluster_id);

        for (auto &boundary : cbv) {
            const auto &global_edge_ids =
                decoding_graph_.GetEdgesTouchingVertex(boundary);
            const auto &vertex_ids =
                decoding_graph_.GetVerticesTouchingVertex(boundary);
            size_t num_edges = global_edge_ids.size();
            for (size_t i = 0; i < num_edges; ++i) {
                if (!fully_grown_edges_[global_edge_ids[i]]) {
                    edge_growth_[global_edge_ids[i]] +=
                        edge_growth_increment_[global_edge_ids[i]];

                    cluster_growth_[cluster_id] +=
                        edge_growth_increment_[global_edge_ids[i]];

                    if (edge_growth_[global_edge_ids[i]] >= max_growth_) {
                        fully_grown_edges_[global_edge_ids[i]] = true;
                        // num_fully_grown_edges_[cluster_id]++;

                        if (vertex_to_cluster_id_[vertex_ids[i]] == -1) {
                            vertex_to_cluster_id_[vertex_ids[i]] = cluster_id;
                            cluster_boundary_.Add(cluster_id, vertex_ids[i]);
                            if (decoding_graph_.IsVertexOnBoundary(
                                    vertex_ids[i])) {
                                cluster_parity_[cluster_id] = -1;
                                physical_boundary_vertices_[vertex_ids[i]] =
                                    true;
                                num_physical_boundary_vertices_++;
                            }
                            continue;
                        }
                        possible_edges_to_fuse.emplace_back(global_edge_ids[i]);
                    }
                }
            }
        }
        return possible_edges_to_fuse;
    }
    /**
     * @brief Find the root of the cluster that a vertex belongs to.
     *
     * This method finds the root of the cluster that the given vertex belongs
     * to. It does so by repeatedly following the vertex-to-cluster ID mappings
     * until it reaches the root of the cluster. It returns the ID of the root,
     * or -1 if the vertex does not belong to a cluster.
     *
     * @param vertex_id The ID of the vertex to find the cluster root for.
     *
     * @return The ID of the root of the cluster that the vertex belongs to, or
     * -1 if the vertex does not belong to a cluster.
     */
    int FindClusterRoot(size_t vertex_id) {
        if (vertex_to_cluster_id_[vertex_id] == -1)
            return -1;
        while (vertex_to_cluster_id_.at(vertex_id) != vertex_id) {
            auto old_vertex_id = vertex_id;
            vertex_id = vertex_to_cluster_id_.at(old_vertex_id);
            vertex_to_cluster_id_.at(old_vertex_id) =
                vertex_to_cluster_id_.at(vertex_id);
        }
        return vertex_id;
    }

    /**
     * @brief Merge the boundary vertices of two clusters.
     *
     * This method merges the boundary vertices of two clusters. It adds all of
     * the boundary vertices of cluster y to cluster x, updates the
     * vertex-to-cluster mappings for those vertices, and removes the mapping
     * for cluster y. If a vertex being added to cluster x is already fully
     * grown, it updates the cluster parity accordingly.
     *
     * @param x The ID of the first cluster to merge.
     * @param y The ID of the second cluster to merge.
     */
    void MergeBoundaryVertices_(size_t x, size_t y) {
        for (auto &vertex_y : cluster_boundary_.GetBoundary(y)) {
            const auto &edges =
                decoding_graph_.GetEdgesTouchingVertex(vertex_y);
            if (IsVertexNotFullyGrown(vertex_y)) {
                cluster_boundary_.Add(x, vertex_y);
                vertex_to_cluster_id_[vertex_y] = x;
            }
        }
    }

    /**
     * @brief Remove fully grown boundary vertices from a cluster's boundary
     * vertex set.
     *
     * This method removes any boundary vertices from the given cluster's
     * boundary vertex set that are fully grown (i.e., all edges connected to
     * them are fully grown).
     *
     * @param cluster_id The ID of the cluster whose boundary vertex set to
     * check.
     */
    void CheckBoundaryVertices(size_t cluster_id) {
        auto &&cbv = cluster_boundary_.GetBoundary(cluster_id);
        for (size_t i = 0; i < cbv.size(); i++) {
            if (!IsVertexNotFullyGrown(cbv[i])) {
                cluster_boundary_.Remove(cluster_id, i);
            }
        }
        cluster_boundary_.Defragment(cluster_id);
    }

    /**
     * @brief Merge two clusters.
     *
     * This method merges two clusters together. It updates the
     * vertex-to-cluster mappings for all vertices in the smaller cluster to
     * point to the larger cluster, and merges the boundary vertices and growth
     * rates of the two clusters. If both clusters have even parity, the merged
     * cluster also has even parity; otherwise, it has odd parity.
     *
     * @param x The ID of the first cluster to merge.
     * @param y The ID of the second cluster to merge.
     *
     * @return The ID of the merged cluster.
     */
    size_t MergeClusters(size_t x, size_t y) {
        if (x == y)
            return x;

        if (cluster_boundary_.GetSize(x) < cluster_boundary_.GetSize(y)) {
            std::swap(x, y);
        }

        vertex_to_cluster_id_[y] = x;
        cluster_growth_[x] += cluster_growth_[y];
        // num_fully_grown_edges_[x] += num_fully_grown_edges_[y];

        if (cluster_parity_[x] >= 0 and cluster_parity_[y] >= 0) {
            cluster_parity_[x] += cluster_parity_[y];
        } else {
            cluster_parity_[x] = -1;
        }

        MergeBoundaryVertices_(x, y);
        return x;
    }

    auto GetGrowQueue() const { return grow_queue_; }

    void AddToGrowQueue(size_t cluster_id) {
        auto boundary_size = cluster_boundary_.GetSize(cluster_id);
        auto edge_length_size = cluster_growth_[cluster_id];
        if (vertex_to_cluster_id_[cluster_id] == cluster_id and
            cluster_parity_[cluster_id] % 2 == 1) {
            grow_queue_.push({boundary_size, edge_length_size, cluster_id});
        }
    }

    int GetSmallestClusterWithOddParity() {
        if (grow_queue_.empty()) {
            return -1;
        } else {
            auto top = grow_queue_.top();
            grow_queue_.pop();

            auto top_boundary_size = std::get<0>(top);
            auto top_edge_length_size = std::get<1>(top);
            auto top_cluster_id = std::get<2>(top);

            while (vertex_to_cluster_id_[top_cluster_id] != top_cluster_id or
                   cluster_boundary_.GetSize(top_cluster_id) !=
                       top_boundary_size or
                   cluster_growth_[top_cluster_id] != top_edge_length_size) {

                if (grow_queue_.empty()) {
                    return -1;
                }
                top = grow_queue_.top();
                grow_queue_.pop();

                top_boundary_size = std::get<0>(top);
                top_edge_length_size = std::get<1>(top);
                top_cluster_id = std::get<2>(top);
            }

            return top_cluster_id;
        }
    };

    /**
     * @brief Get a visualizer for a cluster decoder object
     *
     * This function returns a visualizer object that can be used to visualize
     * the clusters of a cluster decoder object. It takes as input a reference
     * to a stabilizer code object and the grid type (either X or Z) that
     * corresponds to the type of stabilizers to visualize. If `annotate` is set
     * to true, the visualization will include annotations on the vertices and
     * edges.
     *
     * @tparam Code A reference to the cluster decoder object
     * @param code A reference to the stabilizer code object
     * @param grid_type The type of stabilizers to visualize (either X or Z)
     * @param annotate If true, the visualization will include annotations
     * @return A visualizer object for the cluster decoder object
     */
    template <typename Code>
    auto GetVisualizer(Code &code, const StabilizerCode::GridType &grid_type,
                       bool annotate = false) const {

        auto lv = code.GetVisualizer(grid_type, annotate);
        const auto &coords = grid_type == StabilizerCode::GridType::Z
                                 ? code.GetZStabilizerCoords()
                                 : code.GetXStabilizerCoords();

        size_t k = 0;
        for (size_t cc = 0; cc < initial_clusters_.size(); cc++) {

            if (vertex_to_cluster_id_[initial_clusters_[cc]] !=
                initial_clusters_[cc])
                continue;

            size_t c = initial_clusters_[cc];

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

                        size_t global_edge =
                            decoding_graph_.GetGlobalEdgeFromLocalEdge(
                                v_stride + e);
                        float edge_growth = edge_growth_[global_edge];
                        float edge_growth_inc =
                            edge_growth_increment_[global_edge];
                        if (edge_growth >= max_growth_) {

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
                            epp.fraction =
                                (edge_growth > 2.0) ? 1.0 : edge_growth / 2.0;
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
};
}; // namespace Plaquette
