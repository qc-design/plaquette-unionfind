#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Plaquette {

namespace {

std::vector<std::string> color_map = {
    "#FF0000", // Red
    "#FFC300", // Yellow
    "#00FF00", // Green
    "#00FFFF", // Cyan
    "#0000FF", // Blue
    "#FF00FF", // Magenta
    "#FF5733", // Orange
    "#8B00FF", // Purple
    "#FFD700", // Gold
    "#00FF7F", // Spring Green
    "#FF1493", // Deep Pink
    "#00BFFF", // Deep Sky Blue
    "#FFA500", // Orange Red
    "#008000", // Dark Green
    "#FF69B4", // Hot Pink
    "#00FA9A", // Medium Spring Green
    "#8FBC8F", // Dark Sea Green
    "#FF4500", // Orange
    "#40E0D0", // Turquoise
    "#C71585"  // Medium Violet Red
};

std::vector<std::string> marker_styles = {
    ".", // Point marker
    ",", // Pixel marker
    "o", // Circle marker
    "v", // Downward-pointing triangle marker
    "^", // Upward-pointing triangle marker
    "<", // Leftward-pointing triangle marker
    ">", // Rightward-pointing triangle marker
    "1", // Downward-pointing triangle marker
    "2", // Upward-pointing triangle marker
    "3", // Leftward-pointing triangle marker
    "4", // Rightward-pointing triangle marker
    "s", // Square marker
    "p", // Pentagon marker
    "*", // Star marker
    "h", // Hexagon1 marker
    "H", // Hexagon2 marker
    "+", // Plus marker
    "x", // Cross (X) marker
    "D", // Diamond marker
    "d"  // Thin diamond marker
};

std::vector<std::string> line_styles = {
    "-",  // Solid line
    "--", // Dashed line
    "-.", // Dash-dot line
    ":",  // Dotted line
};

}; // namespace

std::string GetHexColor(size_t id) { return color_map[id % color_map.size()]; }
std::string GetMarkerStyle(size_t id) {
    return marker_styles[id % marker_styles.size()];
}
std::string GetLineStyle(size_t id) {
    return line_styles[id % line_styles.size()];
}

struct VertexPrintProps {

    std::string label;
    std::string color;
    std::string fillstyle;
    int markersize;
    std::string marker;
    std::string annotation;

    std::pair<float, float> vertex;

    VertexPrintProps()
        : label("Vertex"), color("red"), fillstyle("full"), markersize(10),
          marker("o"), annotation("") {}
};

struct EdgePrintProps {

    std::string label;
    std::string color;
    std::string linestyle;
    float linewidth;
    float alpha;
    float fraction;
    std::string annotation;

    std::pair<float, float> vertex_0;
    std::pair<float, float> vertex_1;

    EdgePrintProps()
        : label("Edge"), color("blue"), linestyle("solid"), linewidth(1),
          alpha(1.0f), fraction(1.0f), annotation("") {}
};

/**
 * @class LatticeVisualizer
 *
 * @brief A class that provides functionality to visualize a lattice.
 *
 * This class stores a list of vertex and edge properties, and can print out
 * these properties to an output stream in a format suitable for visualizing a
 * lattice. The PrintVertex and PrintEdge functions take in a set of print
 * properties, and print out the corresponding vertex or edge with these
 * properties. The PrintVertices and PrintEdges functions iterate through the
 * stored lists of vertex and edge properties, and print out each vertex or edge
 * with its corresponding properties.
 *
 * @note This class assumes that the lattice is represented as a set of vertices
 * and edges, with each vertex and edge having its own set of properties.
 */
class LatticeVisualizer {

  private:
    std::vector<VertexPrintProps>
        vertex_props_; ///< A vector of vertex print properties
    std::vector<EdgePrintProps>
        edge_props_; ///< A vector of edge print properties

  public:
    /**
     * @brief Add a set of vertex print properties to the list of stored vertex
     * properties.
     *
     * @param props A reference to a VertexPrintProps object representing the
     * properties of the vertex to be added.
     */
    void AddVertexProps(const VertexPrintProps &props) {
        vertex_props_.push_back(props);
    }

    /**
     * @brief Add a set of edge print properties to the list of stored edge
     * properties.
     *
     * @param props A reference to a EdgePrintProps object representing the
     * properties of the edge to be added.
     */
    void AddEdgeProps(const EdgePrintProps &props) {
        edge_props_.push_back(props);
    }

    /**
     * @brief Print out a single vertex to an output stream with the specified
     * print properties.
     *
     * @param out The output stream to print to.
     * @param printprops A reference to a VertexPrintProps object representing
     * the properties of the vertex to be printed.
     */
    void PrintVertex(std::ostream &out, const VertexPrintProps &printprops) {

        out << printprops.vertex.first << " " << printprops.vertex.second << " "
            << printprops.label << " " << printprops.marker << " "
            << printprops.color << " " << printprops.markersize << " "
            << printprops.fillstyle << " " << printprops.annotation
            << std::endl;
    }

    /**
     * @brief Print out a single edge to an output stream with the specified
     * print properties.
     *
     * @param out The output stream to print to.
     * @param printprops A reference to a EdgePrintProps object representing the
     * properties of the edge to be printed.
     */
    void PrintEdge(std::ostream &out, const EdgePrintProps &printprops) {

        out << printprops.vertex_0.first << " " << printprops.vertex_1.first
            << " " << printprops.vertex_0.second << " "
            << printprops.vertex_1.second << " " << printprops.label << " "
            << printprops.linestyle << " " << printprops.color << " "
            << printprops.linewidth << " " << printprops.alpha << " "
            << printprops.fraction << " " << printprops.annotation << std::endl;
    }
    /**
     * @brief Print out all stored vertices to an output stream with their
     * corresponding print properties.
     *
     * This function iterates through the list of stored vertex properties and
     * prints out each vertex with its corresponding properties to the specified
     * output stream.
     *
     * @param out The output stream to print to.
     */
    void PrintVertices(std::ostream &out) {
        for (const auto &props : vertex_props_) {
            PrintVertex(out, props);
        }
    }
    /**
     * @brief Print out all stored edges to an output stream with their
     * corresponding print properties.
     *
     * This function iterates through the list of stored edge properties and
     * prints out each edge with its corresponding properties to the specified
     * output stream.
     *
     * @param out The output stream to print to.
     */
    void PrintEdges(std::ostream &out) {
        for (const auto &props : edge_props_) {
            PrintEdge(out, props);
        }
    }
};

/**
 * @class LatticeVisualizerDB
 *
 * @brief A class that manages a set of LatticeVisualizer objects.
 *
 * This class provides functionality for initializing and managing a set of
 * LatticeVisualizer objects. Each LatticeVisualizer object represents a lattice
 * that can be visualized with a specific set of properties. This class uses an
 * unordered_map to store the set of LatticeVisualizer objects, with each object
 * being associated with a unique string identifier.
 *
 * @note This class assumes that the lattice is represented as a set of vertices
 * and edges, with each vertex and edge having its own set of properties.
 */
class LatticeVisualizerDB {

  private:
    std::unordered_map<std::string, LatticeVisualizer>
        visualizers_; ///< An unordered map of LatticeVisualizer objects, with
                      ///< each object associated with a unique string
                      ///< identifier.

  public:
    /**
     * @brief Default constructor for the LatticeVisualizerDB class.
     */
    LatticeVisualizerDB() = default;

    /**
     * @brief Add a new LatticeVisualizer object with the specified name.
     *
     * LatticeVisualizer object is added to the unordered map of
     * LatticeVisualizer objects managed by this class.
     *
     * @param name A string identifier for the new LatticeVisualizer object.
     */
    void
    AddVisualizer(const std::string &name,
                  const LatticeVisualizer &visualizer = LatticeVisualizer()) {
        visualizers_[name] = visualizer;
    }

    /**
     * @brief Returns the visualizer with the given name.
     *
     * @param name The name of the visualizer to retrieve.
     * @return The visualizer with the given name.
     *
     * @throws std::out_of_range if the given name is not a valid visualizer
     * name.
     */
    auto GetVisualizer(const std::string &name) const {
        return visualizers_.at(name);
    }

    /**
     * @brief Add a set of vertex print properties to the LatticeVisualizer
     * object with the specified identifier.
     *
     * This function adds a set of vertex print properties to the
     * LatticeVisualizer object with the specified string identifier.
     *
     * @param id A string identifier for the LatticeVisualizer object to add the
     * vertex print properties to.
     * @param props A reference to a VertexPrintProps object representing the
     * properties of the vertex to be added.
     */
    void AddVertexProps(const std::string &id, const VertexPrintProps &props) {
        visualizers_[id].AddVertexProps(props);
    }

    /**
     * @brief Add a set of edge print properties to the LatticeVisualizer object
     * with the specified identifier.
     *
     * This function adds a set of edge print properties to the
     * LatticeVisualizer object with the specified string identifier.
     *
     * @param id A string identifier for the LatticeVisualizer object to add the
     * edge print properties to.
     * @param props A reference to an EdgePrintProps object representing the
     * properties of the edge to be added.
     */
    void AddEdgeProps(const std::string &id, const EdgePrintProps &props) {
        visualizers_[id].AddEdgeProps(props);
    }

    /**
     * @brief Print out all stored vertices and edges for each LatticeVisualizer
     * object to separate output files.
     *
     * This function iterates through the unordered map of LatticeVisualizer
     * objects managed by this class, and prints out the stored vertices and
     * edges for each object to separate output files. The name of each output
     * file is constructed by appending "-vertices.dat" or "-edges.dat" to the
     * string identifier of the corresponding LatticeVisualizer object.
     *
     */
    void PrintToFile() {
        for (auto &p : visualizers_) {
            std::ofstream vertices_file(p.first + "-vertices.dat");
            assert(vertices_file.is_open());
            std::ofstream edges_file(p.first + "-edges.dat");
            assert(edges_file.is_open());
            p.second.PrintVertices(vertices_file);
            p.second.PrintEdges(edges_file);
        }
    }

    void Plot(const std::string &id) {

        std::ifstream plot_file("plot.py");
        if (!plot_file.good()) { // Check if plot_file exists and is accessible
            std::cerr << "Error: plot_file plot.py does not exist or cannot be "
                         "accessed.\n";
            std::cerr
                << "Please copy plot.py over to the directory of the executable"
                << std::endl;
            std::exit(EXIT_FAILURE); // Exit program with error code
        }

        auto p = GetVisualizer(id);
        std::ofstream vertices_file(id + "-vertices.dat");
        assert(vertices_file.is_open());
        std::ofstream edges_file(id + "-edges.dat");
        assert(edges_file.is_open());
        p.PrintVertices(vertices_file);
        p.PrintEdges(edges_file);
        std::string command = "python plot.py --edges " + id +
                              "-edges.dat --vertices " + id +
                              "-vertices.dat --output " + id + ".png --title " +
                              id + " --show_plot 1";
        std::cout << command << std::endl;
        std::system(command.c_str());
    }
};

}; // namespace Plaquette
