#pragma once

#include "DecodingGraph.hpp"
#include "Types.hpp"
#include <vector>

namespace Plaquette {

void GetSpanningTreeDFS(
    const DecodingGraph &decoding_graph,
    const Types::UnorderedMap<size_t, Types::UnorderedSet<size_t>>
        &erasure_adj_list,
    std::vector<bool> &visited, std::vector<size_t> &spanning_tree,
    std::vector<size_t> &vertex_count, size_t seed) {

    visited[seed] = true;
    for (const auto &neighbor : erasure_adj_list.at(seed)) {
        if (!visited[neighbor]) {
            auto edge_id =
                decoding_graph.GetEdgeFromVertexPair({seed, neighbor});
            spanning_tree.push_back(edge_id);
            vertex_count[seed] += 1;
            vertex_count[neighbor] += 1;
            GetSpanningTreeDFS(decoding_graph, erasure_adj_list, visited,
                               spanning_tree, vertex_count, neighbor);
        }
    }
}

auto GetSpanningForestDFS(
    const DecodingGraph &decoding_graph,
    const Types::UnorderedMap<size_t, Types::UnorderedSet<size_t>>
        &adjacency_list) {
    std::vector<bool> visited(decoding_graph.GetNumVertices(), false);
    std::vector<size_t> spanning_forest;
    std::vector<size_t> vertex_count(decoding_graph.GetNumVertices(), 0);

    for (auto &i : adjacency_list) {
        if (!visited[i.first]) {
            GetSpanningTreeDFS(decoding_graph, adjacency_list, visited,
                               spanning_forest, vertex_count, i.first);
        }
    }

    return std::make_pair(spanning_forest, vertex_count);
}

void GetSpanningTreeDFSSeeded(
    const DecodingGraph &decoding_graph,
    const Types::UnorderedMap<size_t, Types::UnorderedSet<size_t>>
        &erasure_adj_list,
    std::vector<bool> &visited, std::vector<size_t> &spanning_tree,
    std::vector<size_t> &vertex_count, size_t seed,
    const Types::UnorderedSet<size_t> &seeds, size_t &visited_size) {

    visited[seed] = true;
    visited_size += 1;
    for (int neighbor : erasure_adj_list.at(seed)) {
        if (!visited[neighbor] and !seeds.contains(neighbor)) {
            auto edge_id =
                decoding_graph.GetEdgeFromVertexPair({seed, neighbor});
            spanning_tree.push_back(edge_id);
            vertex_count[seed] += 1;
            vertex_count[neighbor] += 1;
            GetSpanningTreeDFSSeeded(decoding_graph, erasure_adj_list, visited,
                                     spanning_tree, vertex_count, neighbor,
                                     seeds, visited_size);
        }
    }
}

auto GetSpanningForestDFSSeeded(
    const DecodingGraph &decoding_graph,
    const Types::UnorderedMap<size_t, Types::UnorderedSet<size_t>>
        &adjacency_list,
    const Types::UnorderedSet<size_t> &seeds = {}) {

    std::vector<bool> visited(decoding_graph.GetNumVertices(), false);
    std::vector<size_t> spanning_forest;
    std::vector<size_t> vertex_count(decoding_graph.GetNumVertices(), 0);
    size_t visited_size = 0;

    for (auto &i : seeds) {
        if (!visited[i]) {
            GetSpanningTreeDFSSeeded(decoding_graph, adjacency_list, visited,
                                     spanning_forest, vertex_count, i, seeds,
                                     visited_size);
        }
    }

    if (visited_size != adjacency_list.size()) {
        for (auto &i : adjacency_list) {
            if (!visited[i.first]) {
                GetSpanningTreeDFSSeeded(decoding_graph, adjacency_list,
                                         visited, spanning_forest, vertex_count,
                                         i.first, seeds, visited_size);
            }
        }
    }

    return std::make_pair(spanning_forest, vertex_count);
}

Types::UnorderedMap<size_t, Types::UnorderedSet<size_t>>
GetAdjacencyList(const DecodingGraph &decoding_graph,
                 const std::vector<bool> &edge_list) {
    Types::UnorderedMap<size_t, Types::UnorderedSet<size_t>> adjacency_list;

    for (size_t e = 0; e < edge_list.size(); e++) {
        if (!edge_list[e]) {
            continue;
        }

        auto &vertices = decoding_graph.GetVerticesConnectedByEdge(e);
        if (adjacency_list.contains(vertices.first)) {
            adjacency_list[vertices.first].insert(vertices.second);
        } else {
            adjacency_list[vertices.first] = {vertices.second};
        }

        if (adjacency_list.contains(vertices.second)) {
            adjacency_list[vertices.second].insert(vertices.first);
        } else {
            adjacency_list[vertices.second] = {vertices.first};
        }
    }
    return adjacency_list;
}

void GetSpanningTreeCacheFriendly(const DecodingGraph &decoding_graph,
                                  const std::vector<bool> &edge_list,
                                  std::vector<bool> &visited,
                                  // std::vector<bool> & evisited,
                                  std::vector<size_t> &spanning_tree,
                                  std::vector<size_t> &vertex_count,
                                  size_t seed) {
    visited[seed] = true;
    const auto &vertex_vneighbours =
        decoding_graph.GetVerticesTouchingVertex(seed);
    const auto &vertex_eneighbours =
        decoding_graph.GetEdgesTouchingVertex(seed);

    for (size_t i = 0; i < vertex_vneighbours.size(); i++) {
        const auto &vneighbour = vertex_vneighbours[i];
        const auto &eneighbour = vertex_eneighbours[i];
        if (edge_list[eneighbour] /*&& !evisited[eneighbour]*/ &&
            !visited[vneighbour]) {
            spanning_tree.push_back(eneighbour);
            // evisited[eneighbour] = true;
            ++vertex_count[seed];
            ++vertex_count[vneighbour];
            GetSpanningTreeCacheFriendly(decoding_graph, edge_list, visited,
                                         /*evisited,*/
                                         spanning_tree, vertex_count,
                                         vneighbour);
        }
    }
}

auto GetSpanningForestCacheFriendly(const DecodingGraph &decoding_graph,
                                    const std::vector<bool> &edge_list) {

    size_t num_vertices = decoding_graph.GetNumVertices();
    // size_t num_edges = decoding_graph.GetNumEdges();
    std::vector<bool> visited(num_vertices, false);
    // std::vector<bool> evisited (num_edges, false);
    std::vector<size_t> vertex_count(num_vertices, 0);
    std::vector<size_t> spanning_forest;

    for (size_t e = 0; e < edge_list.size(); e++) {
        if (edge_list[e] /*and !evisited[e]*/) {
            const auto &[v1, v2] = decoding_graph.GetVerticesConnectedByEdge(e);
            if (!visited[v1]) {
                // std::cout << "v1: " << v1 << std::endl;
                GetSpanningTreeCacheFriendly(decoding_graph, edge_list, visited,
                                             /*evisited,*/ spanning_forest,
                                             vertex_count, v1);
            }
            if (!visited[v2]) {
                // std::cout << "v2: " << v2 << std::endl;
                GetSpanningTreeCacheFriendly(decoding_graph, edge_list, visited,
                                             /* evisited,*/ spanning_forest,
                                             vertex_count, v2);
            }
        }
    }

    return std::make_pair(spanning_forest, vertex_count);
}

void GetSpanningTreeCacheFriendlySeeded(const DecodingGraph &decoding_graph,
                                        const std::vector<bool> &edge_list,
                                        std::vector<bool> &visited,
                                        // std::vector<bool> & evisited,
                                        std::vector<size_t> &spanning_tree,
                                        std::vector<size_t> &vertex_count,
                                        size_t seed,
                                        const std::vector<bool> &seeds) {
    visited[seed] = true;
    const auto &vertex_vneighbours =
        decoding_graph.GetVerticesTouchingVertex(seed);
    const auto &vertex_eneighbours =
        decoding_graph.GetEdgesTouchingVertex(seed);

    for (size_t i = 0; i < vertex_vneighbours.size(); i++) {
        const auto &vneighbour = vertex_vneighbours[i];
        const auto &eneighbour = vertex_eneighbours[i];
        if (edge_list[eneighbour] /*&& !evisited[eneighbour]*/ &&
            !visited[vneighbour] and !seeds[vneighbour]) {
            spanning_tree.push_back(eneighbour);
            // evisited[eneighbour] = true;
            ++vertex_count[seed];
            ++vertex_count[vneighbour];
            GetSpanningTreeCacheFriendlySeeded(
                decoding_graph, edge_list, visited,
                /*evisited,*/
                spanning_tree, vertex_count, vneighbour, seeds);
        }
    }
}

auto GetSpanningForestCacheFriendlySeeded(const DecodingGraph &decoding_graph,
                                          const std::vector<bool> &edge_list,
                                          const std::vector<bool> &seeds,
                                          size_t seeds_size) {

    size_t num_vertices = decoding_graph.GetNumVertices();
    std::vector<bool> visited(num_vertices, false);
    std::vector<size_t> vertex_count(num_vertices, 0);
    std::vector<size_t> spanning_forest;

    if (seeds_size != 0) {
        for (size_t i = 0; i < seeds.size(); i++) {
            if (seeds[i] and !visited[i]) {
                GetSpanningTreeCacheFriendlySeeded(decoding_graph, edge_list,
                                                   visited, spanning_forest,
                                                   vertex_count, i, seeds);
            }
        }
    }

    for (size_t e = 0; e < edge_list.size(); e++) {
        if (edge_list[e]) {
            const auto &[v1, v2] = decoding_graph.GetVerticesConnectedByEdge(e);
            if (!visited[v1]) {
                GetSpanningTreeCacheFriendlySeeded(
                    decoding_graph, edge_list, visited,
                    /*evisited,*/ spanning_forest, vertex_count, v1, seeds);
            }
            if (!visited[v2]) {
                GetSpanningTreeCacheFriendlySeeded(
                    decoding_graph, edge_list, visited,
                    /* evisited,*/ spanning_forest, vertex_count, v2, seeds);
            }
        }
    }

    return std::make_pair(spanning_forest, vertex_count);
}

}; // namespace Plaquette
