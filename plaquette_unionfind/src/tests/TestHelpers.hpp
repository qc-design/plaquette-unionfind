#pragma once

#include "DecodingGraph.hpp"
#include <vector>

namespace Plaquette {

auto MeasureSyndrome(const DecodingGraph &decoding_graph,
                     const std::vector<bool> &errors) {

    std::vector<bool> syndrome(decoding_graph.GetNumVertices(), false);
    for (size_t v = 0; v < decoding_graph.GetNumVertices(); ++v) {
        if (decoding_graph.IsVertexOnBoundary(v)) {
            continue;
        }

        const auto &edges = decoding_graph.GetEdgesTouchingVertex(v);
        size_t toggle = 0;

        for (size_t e = 0; e < edges.size(); e++) {
            if (errors[edges[e]]) {
                toggle++;
            }
        }

        if (toggle % 2 == 1) {
            syndrome[v] = true;
        }
    }

    return syndrome;
}

}; // namespace Plaquette
