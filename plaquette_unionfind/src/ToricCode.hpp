#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "DecodingGraph.hpp"
#include "LatticeVisualizer.hpp"
#include "StabilizerCode.hpp"

namespace Plaquette {

/**
 * @brief A class representing a toric code.
 *
 * A toric code is a two-dimensional topological code that encodes qubits on a
 * torus. This class inherits from the StabilizerCode class and defines
 * additional methods specific to the toric code, such as coordinate
 * transformations, lattice building, and visualization methods.
 *
 */
class ToricCode : public StabilizerCode {

  private:
    size_t lattice_size_; ///< The size of the lattice.
    std::unordered_map<int, size_t>
        linearized_qubit_coord_to_edge_id_; ///< A map from linearized qubit
                                            ///< coordinates to edge IDs.
    std::unordered_map<int, size_t>
        linearized_x_stab_coord_to_vertex_id_; ///< A map from linearized
                                               ///< X-stabilizer coordinates to
                                               ///< vertex IDs.
    std::unordered_map<int, size_t>
        linearized_z_stab_coord_to_vertex_id_; ///< A map from linearized
                                               ///< Z-stabilizer coordinates to
                                               ///< vertex IDs.

  public:
    /**
     * @brief Get the modulo of a coordinate.
     *
     * @param coord The coordinate to be modified.
     * @return An integer representing the modified coordinate.
     */
    int ModuloCoord(int coord) {
        if (coord < 0) {
            return 2 * lattice_size_ + coord;
        } else if (coord >= 2 * lattice_size_) {
            return coord - 2 * lattice_size_;
        } else {
            return coord;
        }
    }

    bool IsPeriodic() const { return true; }

    /**
     * @brief Linearize a pair of coordinates.
     *
     * @param coord A pair of coordinates to be linearized.
     * @return An integer representing the linearized coordinates.
     */
    int LinearizeCoord(std::pair<int, int> coord) {
        auto mod_first = ModuloCoord(coord.first);
        auto mod_second = ModuloCoord(coord.second);
        return mod_first + mod_second * 2 * lattice_size_;
    }

    /**
     * @brief Delinearize a linearized coordinate.
     *
     * @param linear_coord The linearized coordinate to be delinearized.
     * @return A pair of coordinates.
     */
    std::pair<int, int> DelinearizeCoord(int linear_coord) {
        int x = linear_coord % 2 * lattice_size_;
        int y = linear_coord / 2 * lattice_size_;
        return std::make_pair(x, y);
    }

    /**
     * @brief Get the number of qubits in the code.
     *
     * @return The number of qubits in the code.
     */
    size_t GetNumOfQubits() {
        return linearized_qubit_coord_to_edge_id_.size();
    }

    /**
     * @brief Construct a toric code of a given size.
     *
     * @param lattice_size The size of the toric lattice.
     */
    ToricCode(size_t lattice_size) {
        lattice_size_ = lattice_size;
        size_t num_vertices = lattice_size * lattice_size;
        std::vector<bool> vertex_boundary_type(num_vertices, 0);

        // Build lattice
        for (int y = 2 * lattice_size - 1; y >= 0; y--) {
            for (int x = 0; x < 2 * lattice_size; x++) {
                auto coord = std::make_pair(x, y);
                auto linear_coord = LinearizeCoord(coord);
                if (x % 2 == 0) {
                    if (y % 2 == 0) {
                        // X stabilizer
                        x_stabilizer_coords_.push_back(std::make_pair(x, y));
                        linearized_x_stab_coord_to_vertex_id_[linear_coord] =
                            x_stabilizer_coords_.size() - 1;
                    } else {
                        // qubit
                        qubit_coords_.push_back(std::make_pair(x, y));
                        linearized_qubit_coord_to_edge_id_[linear_coord] =
                            qubit_coords_.size() - 1;
                    }
                } else {
                    if (y % 2 == 0) {
                        // qubit
                        qubit_coords_.push_back(std::make_pair(x, y));
                        linearized_qubit_coord_to_edge_id_[linear_coord] =
                            qubit_coords_.size() - 1;
                    } else {
                        // Z stabilizer
                        z_stabilizer_coords_.push_back(std::make_pair(x, y));
                        linearized_z_stab_coord_to_vertex_id_[linear_coord] =
                            z_stabilizer_coords_.size() - 1;
                    }
                }
            }
        }

        std::vector<std::pair<size_t, size_t>> x_dgraph_edges(
            qubit_coords_.size());
        std::vector<std::pair<size_t, size_t>> z_dgraph_edges(
            qubit_coords_.size());

        for (int i = 0; i < qubit_coords_.size(); i++) {
            auto qub = qubit_coords_[i];
            std::vector<std::pair<int, int>> dx = {
                {-1, 0}, {1, 0}, {0, 1}, {0, -1}};
            std::vector<size_t> x_vertices;
            std::vector<size_t> z_vertices;

            for (size_t j = 0; j < dx.size(); j++) {
                std::pair<int, int> stab_coord;
                stab_coord.first = qub.first + dx[j].first;
                stab_coord.second = qub.second + dx[j].second;

                if (linearized_x_stab_coord_to_vertex_id_.contains(
                        LinearizeCoord(stab_coord))) {
                    x_vertices.push_back(
                        linearized_x_stab_coord_to_vertex_id_[LinearizeCoord(
                            stab_coord)]);
                }
                if (linearized_z_stab_coord_to_vertex_id_.contains(
                        LinearizeCoord(stab_coord))) {
                    z_vertices.push_back(
                        linearized_z_stab_coord_to_vertex_id_[LinearizeCoord(
                            stab_coord)]);
                }
            }
            if (x_vertices.size()) {
                assert(x_vertices.size() == 2);
                x_dgraph_edges[i] =
                    std::make_pair(x_vertices[0], x_vertices[1]);
            }
            if (z_vertices.size()) {
                assert(z_vertices.size() == 2);
                z_dgraph_edges[i] =
                    std::make_pair(z_vertices[0], z_vertices[1]);
            }
        }

        x_stabilizer_decoding_graph_ =
            DecodingGraph(num_vertices, x_dgraph_edges, vertex_boundary_type);
        z_stabilizer_decoding_graph_ =
            DecodingGraph(num_vertices, z_dgraph_edges, vertex_boundary_type);

        logical_x_qubits_.resize(2, std::vector<size_t>(lattice_size));
        for (size_t i = 0; i < lattice_size; i++) {
            logical_x_qubits_[0][i] =
                2 * lattice_size * lattice_size - 1 - i - lattice_size;
        }
        for (size_t i = 0; i < lattice_size; i++) {
            logical_x_qubits_[1][i] = 2 * i * lattice_size + lattice_size;
        }

        logical_z_qubits_.resize(2, std::vector<size_t>(lattice_size));
        for (size_t i = 0; i < lattice_size; i++) {
            logical_z_qubits_[0][i] = 2 * lattice_size * lattice_size - 1 - i;
        }
        for (size_t i = 0; i < lattice_size; i++) {
            logical_z_qubits_[1][i] = 2 * i * lattice_size;
        }
    }

    void FixEdgeCoordsForVisual(std::pair<float, float> &vertex_0,
                                std::pair<float, float> &vertex_1) const {

        if (std::abs(vertex_0.first - vertex_1.first) > 2) {

            if (vertex_0.first > vertex_1.first) {
                vertex_0.first = vertex_1.first - 2;
            } else {
                vertex_1.first = vertex_0.first - 2;
            }
        }
        if (std::abs(vertex_0.second - vertex_1.second) > 2) {
            if (vertex_0.second > vertex_1.second) {
                vertex_0.second = vertex_1.second - 2;
            } else {
                vertex_1.second = vertex_0.second - 2;
            }
        }
    }

    /**
     * @brief Get a LatticeVisualizer object representing the stabilizer code.
     *
     * This function returns a LatticeVisualizer object that represents the
     * stabilizer code by adding vertices and edges to the visualizer according
     * to the type of grid. If the grid is of type Qubit, it adds vertices for
     * each qubit, X stabilizer, and Z stabilizer to the visualizer. If the grid
     * is of type Z, it adds vertices for each Z stabilizer and edges connecting
     * neighboring stabilizers. If the grid is of type X, it adds vertices for
     * each X stabilizer and edges connecting neighboring stabilizers.
     *
     * @param grid_type The type of the grid to use for the stabilizer code.
     * @param annotate A boolean flag indicating whether to annotate the
     * vertices with their indices.
     * @return A LatticeVisualizer object representing the stabilizer code.
     */
    auto GetVisualizer(const StabilizerCode::GridType &grid_type,
                       bool annotate = false,
                       bool display_logical = false) const {

        LatticeVisualizer visualizer;
        if (grid_type == StabilizerCode::GridType::Qubit) {

            const auto &qubit_coords = GetQubitCoords();
            const auto &x_stab_coords = GetXStabilizerCoords();
            const auto &z_stab_coords = GetZStabilizerCoords();

            for (size_t i = 0; i < qubit_coords.size(); i++) {
                VertexPrintProps qubit_vertex_props;
                qubit_vertex_props.vertex = qubit_coords[i];
                qubit_vertex_props.marker = "o";
                qubit_vertex_props.annotation =
                    (!annotate) ? "" : std::to_string(i);
                qubit_vertex_props.color = "black";
                qubit_vertex_props.markersize = 10;
                qubit_vertex_props.fillstyle = "full";
                qubit_vertex_props.label = "qubit";
                visualizer.AddVertexProps(qubit_vertex_props);
            }

            for (size_t i = 0; i < x_stab_coords.size(); i++) {
                VertexPrintProps x_stab_vertex_props;
                x_stab_vertex_props.vertex = x_stab_coords[i];
                x_stab_vertex_props.marker = "D";
                x_stab_vertex_props.annotation =
                    (!annotate) ? "" : std::to_string(i);
                x_stab_vertex_props.color = "black";
                x_stab_vertex_props.markersize = 10;
                x_stab_vertex_props.fillstyle = "full";
                x_stab_vertex_props.label = "x_stab";
                visualizer.AddVertexProps(x_stab_vertex_props);
            };

            for (size_t i = 0; i < z_stab_coords.size(); i++) {
                VertexPrintProps z_stab_vertex_props;
                z_stab_vertex_props.vertex = z_stab_coords[i];
                z_stab_vertex_props.marker = "s";
                z_stab_vertex_props.annotation =
                    (!annotate) ? "" : std::to_string(i);
                z_stab_vertex_props.color = "black";
                z_stab_vertex_props.markersize = 10;
                z_stab_vertex_props.fillstyle = "full";
                z_stab_vertex_props.label = "z_stab";
                visualizer.AddVertexProps(z_stab_vertex_props);
            };

            if (display_logical == true) {

                const auto &logical_x_qubits = GetLogicalXQubits();
                const auto &logical_z_qubits = GetLogicalZQubits();

                for (size_t i = 0; i < logical_x_qubits.size(); i++) {
                    for (size_t j = 0; j < logical_x_qubits[i].size(); j++) {
                        VertexPrintProps logical_x_vertex_props;
                        logical_x_vertex_props.vertex =
                            qubit_coords[logical_x_qubits[i][j]];
                        logical_x_vertex_props.marker = "x";
                        // logical_x_vertex_props.annotation = (!annotate) ? ""
                        // : std::to_string(logical_x_qubits[i][j]);
                        logical_x_vertex_props.color = GetHexColor(i);
                        logical_x_vertex_props.markersize = 20;
                        logical_x_vertex_props.fillstyle = "full";
                        logical_x_vertex_props.label =
                            "logical_x" + std::to_string(i);
                        visualizer.AddVertexProps(logical_x_vertex_props);
                    }
                }

                for (size_t i = 0; i < logical_z_qubits.size(); i++) {
                    for (size_t j = 0; j < logical_z_qubits[i].size(); j++) {
                        VertexPrintProps logical_z_vertex_props;
                        logical_z_vertex_props.vertex =
                            qubit_coords[logical_z_qubits[i][j]];
                        logical_z_vertex_props.marker = "x";
                        // logical_z_vertex_props.annotation = (!annotate) ? ""
                        // : std::to_string(logical_z_qubits[i][j]);
                        logical_z_vertex_props.color =
                            GetHexColor(i + logical_x_qubits.size());
                        logical_z_vertex_props.markersize = 10;
                        logical_z_vertex_props.fillstyle = "full";
                        logical_z_vertex_props.label =
                            "logical_z_" + std::to_string(i);
                        visualizer.AddVertexProps(logical_z_vertex_props);
                    }
                }
            }

        } else if (grid_type == StabilizerCode::GridType::Z) {
            const auto &z_stab_coords = GetZStabilizerCoords();
            const auto &z_stab_decoding_graph = GetZStabilizerDecodingGraph();

            for (size_t v = 0; v < z_stab_decoding_graph.GetNumVertices();
                 v++) {
                VertexPrintProps z_stab_vertex_props;
                z_stab_vertex_props.vertex = z_stab_coords[v];
                z_stab_vertex_props.marker = "o";
                z_stab_vertex_props.annotation =
                    (!annotate) ? "" : std::to_string(v);
                z_stab_vertex_props.color = "black";
                z_stab_vertex_props.markersize = 10;
                z_stab_vertex_props.fillstyle = "full";
                z_stab_vertex_props.label = "z_stab";
                visualizer.AddVertexProps(z_stab_vertex_props);
            };

            for (size_t e = 0; e < z_stab_decoding_graph.GetNumEdges(); e++) {
                const auto &vertices =
                    z_stab_decoding_graph.GetVerticesConnectedByEdge(e);

                EdgePrintProps z_stab_edge_props;
                z_stab_edge_props.vertex_0 = z_stab_coords[vertices.first];
                z_stab_edge_props.vertex_1 = z_stab_coords[vertices.second];
                z_stab_edge_props.linestyle = "-";
                z_stab_edge_props.label = "z_stab_edge";
                z_stab_edge_props.annotation =
                    (!annotate) ? "" : std::to_string(e);

                if (std::abs(z_stab_edge_props.vertex_0.first -
                             z_stab_edge_props.vertex_1.first) > 2) {

                    if (z_stab_edge_props.vertex_0.first >
                        z_stab_edge_props.vertex_1.first) {
                        z_stab_edge_props.vertex_0.first =
                            z_stab_edge_props.vertex_1.first - 2;
                    } else {
                        z_stab_edge_props.vertex_1.first =
                            z_stab_edge_props.vertex_0.first - 2;
                    }
                    z_stab_edge_props.linestyle = "-.";
                    z_stab_edge_props.label = "z_stab_edge_periodic";
                }
                if (std::abs(z_stab_edge_props.vertex_0.second -
                             z_stab_edge_props.vertex_1.second) > 2) {

                    if (z_stab_edge_props.vertex_0.second >
                        z_stab_edge_props.vertex_1.second) {
                        z_stab_edge_props.vertex_0.second =
                            z_stab_edge_props.vertex_1.second - 2;
                    } else {
                        z_stab_edge_props.vertex_1.second =
                            z_stab_edge_props.vertex_0.second - 2;
                    }
                    z_stab_edge_props.linestyle = "-.";
                    z_stab_edge_props.label = "z_stab_edge_periodic";
                } else if (std::abs(z_stab_edge_props.vertex_0.second -
                                    z_stab_edge_props.vertex_1.second) == 1) {
                    z_stab_edge_props.color = "red";
                } else {
                    z_stab_edge_props.color = "black";
                }
                z_stab_edge_props.color = "blue";
                visualizer.AddEdgeProps(z_stab_edge_props);
            }

            if (display_logical == true) {

                const auto &logical_z_qubits = GetLogicalZQubits();

                for (size_t i = 0; i < logical_z_qubits.size(); i++) {
                    for (size_t j = 0; j < logical_z_qubits[i].size(); j++) {
                        EdgePrintProps logical_z_edge_props;
                        const auto &vertices =
                            z_stab_decoding_graph.GetVerticesConnectedByEdge(
                                logical_z_qubits[i][j]);
                        logical_z_edge_props.vertex_0 =
                            z_stab_coords[vertices.first];
                        logical_z_edge_props.vertex_1 =
                            z_stab_coords[vertices.second];
                        FixEdgeCoordsForVisual(logical_z_edge_props.vertex_0,
                                               logical_z_edge_props.vertex_1);

                        logical_z_edge_props.color = GetHexColor(i);
                        logical_z_edge_props.linewidth = 20;
                        logical_z_edge_props.alpha = .5;
                        logical_z_edge_props.label =
                            "logical_z_" + std::to_string(i);
                        visualizer.AddEdgeProps(logical_z_edge_props);
                    }
                }
            }

        } else {
            const auto &x_stab_coords = GetXStabilizerCoords();
            const auto &x_stab_decoding_graph = GetXStabilizerDecodingGraph();

            for (size_t v = 0; v < x_stab_decoding_graph.GetNumVertices();
                 v++) {
                VertexPrintProps x_stab_vertex_props;
                x_stab_vertex_props.vertex = x_stab_coords[v];
                x_stab_vertex_props.marker = "o";
                x_stab_vertex_props.annotation =
                    (!annotate) ? "" : std::to_string(v);
                x_stab_vertex_props.color = "black";
                x_stab_vertex_props.markersize = 10;
                x_stab_vertex_props.fillstyle = "full";
                x_stab_vertex_props.label = "x_stab";
                visualizer.AddVertexProps(x_stab_vertex_props);
            };

            for (size_t e = 0; e < x_stab_decoding_graph.GetNumEdges(); e++) {
                const auto &vertices =
                    x_stab_decoding_graph.GetVerticesConnectedByEdge(e);

                EdgePrintProps x_stab_edge_props;
                x_stab_edge_props.vertex_0 = x_stab_coords[vertices.first];
                x_stab_edge_props.vertex_1 = x_stab_coords[vertices.second];
                x_stab_edge_props.linestyle = "-";
                x_stab_edge_props.label = "x_stab_edge";
                x_stab_edge_props.annotation =
                    (!annotate) ? "" : std::to_string(e);

                if (std::abs(x_stab_edge_props.vertex_0.first -
                             x_stab_edge_props.vertex_1.first) > 2) {

                    if (x_stab_edge_props.vertex_0.first >
                        x_stab_edge_props.vertex_1.first) {
                        x_stab_edge_props.vertex_0.first =
                            x_stab_edge_props.vertex_1.first - 2;
                    } else {
                        x_stab_edge_props.vertex_1.first =
                            x_stab_edge_props.vertex_0.first - 2;
                    }
                    x_stab_edge_props.linestyle = "-.";
                    x_stab_edge_props.label = "x_stab_edge_periodic";
                }
                if (std::abs(x_stab_edge_props.vertex_0.second -
                             x_stab_edge_props.vertex_1.second) > 2) {

                    if (x_stab_edge_props.vertex_0.second >
                        x_stab_edge_props.vertex_1.second) {
                        x_stab_edge_props.vertex_0.second =
                            x_stab_edge_props.vertex_1.second - 2;
                    } else {
                        x_stab_edge_props.vertex_1.second =
                            x_stab_edge_props.vertex_0.second - 2;
                    }
                    x_stab_edge_props.linestyle = "-.";
                    x_stab_edge_props.label = "x_stab_edge_periodic";
                } else if (std::abs(x_stab_edge_props.vertex_0.second -
                                    x_stab_edge_props.vertex_1.second) == 1) {
                    x_stab_edge_props.color = "red";
                } else {
                    x_stab_edge_props.color = "black";
                }
                x_stab_edge_props.color = "blue";
                visualizer.AddEdgeProps(x_stab_edge_props);
            }

            if (display_logical == true) {
                const auto &logical_x_qubits = GetLogicalXQubits();

                for (size_t i = 0; i < logical_x_qubits.size(); i++) {
                    for (size_t j = 0; j < logical_x_qubits[i].size(); j++) {
                        EdgePrintProps logical_x_edge_props;
                        const auto &vertices =
                            x_stab_decoding_graph.GetVerticesConnectedByEdge(
                                logical_x_qubits[i][j]);
                        logical_x_edge_props.vertex_0 =
                            x_stab_coords[vertices.first];
                        logical_x_edge_props.vertex_1 =
                            x_stab_coords[vertices.second];
                        FixEdgeCoordsForVisual(logical_x_edge_props.vertex_0,
                                               logical_x_edge_props.vertex_1);
                        logical_x_edge_props.color = GetHexColor(i);
                        logical_x_edge_props.linewidth = 20;
                        logical_x_edge_props.label =
                            "logical_x_" + std::to_string(i);
                        visualizer.AddEdgeProps(logical_x_edge_props);
                    }
                }
            }
        }

        return visualizer;
    }
};
}; // namespace Plaquette
