#pragma once

#include <iostream>
#include <limits>
#include <vector>

#include "DecodingGraph.hpp"
#include "Types.hpp"

namespace Plaquette {

/**
 * @brief A class representing a stabilizer code.
 */
class StabilizerCode {

  protected:
    DecodingGraph x_stabilizer_decoding_graph_;
    DecodingGraph z_stabilizer_decoding_graph_;

    std::vector<std::pair<int, int>> x_stabilizer_coords_;
    std::vector<std::pair<int, int>> z_stabilizer_coords_;
    std::vector<std::pair<int, int>> qubit_coords_;

    // In terms of Z stabilizer edge Ids
    std::vector<std::vector<size_t>> logical_x_qubits_;

    // In terms of X stabilizer edge Ids
    std::vector<std::vector<size_t>> logical_z_qubits_;

  public:
    /**
     * @brief Default constructor.
     */
    StabilizerCode() = default;

    /**
     * @brief An enum class representing the channel for measuring a stabilizer
     * code.
     */
    enum class Channel { X, Z };
    enum class Stabilizer { X, Z };

    /**
     * @brief An enum class representing the grid type of a stabilizer code.
     */
    enum class GridType { X, Z, Qubit };

    const auto &GetXStabilizerDecodingGraph() const {
        return x_stabilizer_decoding_graph_;
    }
    const auto &GetZStabilizerDecodingGraph() const {
        return z_stabilizer_decoding_graph_;
    }
    const auto &GetXStabilizerCoords() const { return x_stabilizer_coords_; }
    const auto &GetZStabilizerCoords() const { return z_stabilizer_coords_; }
    const auto &GetQubitCoords() const { return qubit_coords_; }
    const auto &GetLogicalXQubits() const { return logical_x_qubits_; }
    const auto &GetLogicalZQubits() const { return logical_z_qubits_; }

    /**
     * @brief Determines whether the stabilizer code is periodic.
     *
     * @return True if the stabilizer code is periodic, false otherwise.
     */
    bool IsPeriodic() const { return false; }

    /**
     * @brief Calculates the code distance of the stabilizer code.
     *
     * @return The code distance of the stabilizer code.
     */
    size_t GetCodeDistance() {
        // The code distance is the measure of strength of the code, and is the
        // length of the smallest undetectable error chain – that is, the length
        // of the smallest logical operator https://arxiv.org/pdf/1111.4022.pdf
        assert(logical_x_qubits_.size() > 0 || logical_z_qubits_.size() > 0);
        size_t min_size = std::numeric_limits<size_t>::max();
        for (auto &logx : logical_x_qubits_) {
            min_size = min_size < logx.size() ? min_size : logx.size();
        }
        for (auto &logz : logical_z_qubits_) {
            min_size = min_size < logz.size() ? min_size : logz.size();
        }
        return min_size;
    }

    /**
     * @brief Measures the logical operator of the stabilizer code in the
     * specified channel.
     *
     * @param errors The error vector.
     * @param channel The channel (X or Z) to measure.
     * @return True if the measurement outcome is -1, false otherwise.
     */
    bool MeasureLogical(const std::vector<bool> &errors,
                        const Channel &channel) {
        assert(channel == Channel::Z || channel == Channel::X);
        if (channel == Channel::X) {
            for (size_t l = 0; l < logical_x_qubits_.size(); ++l) {
                int measurement = 1;
                for (size_t q = 0; q < logical_x_qubits_[l].size(); ++q) {
                    if (errors[logical_x_qubits_[l][q]]) {
                        measurement *= -1;
                    }
                }
                if (measurement == -1) {
                    return true;
                }
            }
        } else {
            for (size_t l = 0; l < logical_z_qubits_.size(); ++l) {
                int measurement = 1;
                for (size_t q = 0; q < logical_z_qubits_[l].size(); ++q) {
                    if (errors[logical_z_qubits_[l][q]]) {
                        measurement *= -1;
                    }
                }
                if (measurement == -1) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @brief Measures the syndrome of the stabilizer code in the specified
     * channel.
     *
     * @param errors The error vector.
     * @param channel The channel (X or Z) to measure.
     * @return A vector of booleans representing the syndrome.
     */
    std::vector<bool> MeasureSyndrome(const std::vector<bool> &errors,
                                      const Stabilizer &stab) const {

        const auto &decoding_graph = stab == Stabilizer::X
                                         ? x_stabilizer_decoding_graph_
                                         : z_stabilizer_decoding_graph_;

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
};
}; // namespace Plaquette
