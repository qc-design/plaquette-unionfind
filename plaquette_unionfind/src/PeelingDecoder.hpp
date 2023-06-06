#pragma once

#include <deque>
#include <iostream>
#include <stack>
#include <vector>

#include "DecodingGraph.hpp"
#include "LatticeVisualizer.hpp"
#include "SpanningForest.hpp"
#include "Types.hpp"
#include "Utils.hpp"

namespace Plaquette {
namespace Decoders {

class PeelingDecoder {

  public:
    std::vector<bool> Decode(const DecodingGraph &decoding_graph,
                             std::vector<bool> &syndrome,
                             const std::vector<bool> &erasure,
                             const std::vector<bool> &seeds = {},
                             size_t seeds_size = 0) {
        auto &&[tree, vertex_count] =
            (seeds_size == 0)
                ? GetSpanningForestCacheFriendly(decoding_graph, erasure)
                : GetSpanningForestCacheFriendlySeeded(decoding_graph, erasure,
                                                       seeds, seeds_size);
        return PeelForest(decoding_graph, syndrome, tree, vertex_count);
    }

    std::vector<bool> PeelForest(const DecodingGraph &decoding_graph,
                                 std::vector<bool> &syndrome,
                                 const std::vector<size_t> &tree,
                                 std::vector<size_t> &vertex_count) {
        size_t tree_size = tree.size();
        std::vector<bool> error_edges(decoding_graph.GetNumEdges(), false);
        for (size_t j = 0; j < tree_size; ++j) {

            // iterate backwards through the tree
            size_t i = tree_size - j - 1;
            const auto &edge =
                decoding_graph.GetVerticesConnectedByEdge(tree[i]);

            bool swap_uv = vertex_count[edge.first] != 1 ||
                           decoding_graph.IsVertexOnBoundary(edge.first);
            auto &u = swap_uv ? edge.second : edge.first;
            auto &v = swap_uv ? edge.first : edge.second;

            vertex_count[u] -= 1;
            vertex_count[v] -= 1;

            if (syndrome[u]) {
                // add error edge
                error_edges[tree[i]] = true;

                // remove u from syndrome
                syndrome[u] = false;

                // flip v
                syndrome[v] = !syndrome[v];
            }
        }

        return error_edges;
    }
};

}; // namespace Decoders

}; // namespace Plaquette
