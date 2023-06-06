#pragma once
#include <cassert>
#include <iostream>
#include <vector>

namespace Plaquette {
/**
 * @brief A lightweight view of a cluster boundary.
 */
class ClusterBoundary {
  public:
    /**
     * @brief Constructs a ClusterBoundary object from a row vector.
     * @param row The row vector of the boundary.
     * @param start The starting index of the row.
     * @param end The ending index of the row.
     */
    ClusterBoundary(std::vector<int> &row, size_t start, size_t end)
        : boundary(row), start_(start), end_(end) {}

    /**
     * @brief Returns the number of non-zero elements in the row.
     * @return The number of non-zero elements in the row.
     */
    size_t size() const { return end_ - start_; }

    bool contains(int i) const {
        for (size_t j = start_; j < end_; j++) {
            if (boundary[j] == i) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Returns the value at a specific index in the row.
     * @param index The index of the element to retrieve.
     * @return The value at the specified index.
     */
    int operator[](int index) const { return boundary[start_ + index]; }

    /**
     * @brief Returns a reference to the value at a specific index in the row.
     * @param index The index of the element to retrieve.
     * @return A reference to the value at the specified index.
     */
    int &operator[](int index) { return boundary[start_ + index]; }

    /**
     * @brief Returns an iterator to the start of the lightweight view.
     * @return An iterator to the start of the lightweight view.
     */
    std::vector<int>::iterator begin() { return boundary.begin() + start_; }

    /**
     * @brief Returns an iterator to the end of the lightweight view.
     * @return An iterator to the end of the lightweight view.
     */
    std::vector<int>::iterator end() { return boundary.begin() + end_; }

  private:
    /**
     * @brief A reference to the row vector.
     */
    std::vector<int> &boundary;

    /**
     * @brief The starting index of the row.
     */
    size_t start_;

    /**
     * @brief The ending index of the row.
     */
    size_t end_;
};

/**
 * @brief Stores cluster boundaries for efficient boundary computation of
 *        graph partitions.
 */
class ClusterBoundaries {

  private:
    /**
     * @brief The maximum size of each cluster boundary.
     */
    size_t max_boundary_size_;

    /**
     * @brief The vector that stores the cluster boundary data.
     */
    std::vector<int> boundary_;

    /**
     * @brief The vector that stores the cluster strides.
     */
    std::vector<int> cluster_strides_;

    /**
     * @brief The vector that stores the sizes of each cluster boundary.
     */
    std::vector<size_t> boundary_sizes_;

    size_t num_clusters_;

  public:
    ClusterBoundaries() = default;

    /**
     * @brief Constructs a ClusterBoundaries object from a vector of cluster
     *        indices, the number of vertices, and the maximum boundary size.
     * @param clusters A vector of cluster indices.
     * @param num_vertices The number of vertices in the graph.
     * @param max_boundary_size The maximum size of each cluster boundary.
     */
    ClusterBoundaries(
        std::vector<size_t> clusters, size_t num_vertices,
        size_t max_boundary_size,
        const std::vector<std::pair<size_t, size_t>> &initial = {}) {

        max_boundary_size_ = max_boundary_size;
        boundary_ = std::vector<int>(clusters.size() * max_boundary_size, -1);
        boundary_sizes_ = std::vector<size_t>(clusters.size(), 0);
        cluster_strides_ = std::vector<int>(num_vertices, -1);
        num_clusters_ = clusters.size();

        for (size_t i = 0; i < clusters.size(); i++) {
            cluster_strides_[clusters[i]] = i;
        }

        for (auto &i : initial) {
            Add(i.first, i.second);
        }
    }

    ClusterBoundaries(size_t num_vertices, size_t max_boundary_size,
                      size_t scratch_size = 0)

    {
        num_clusters_ = 0;
        max_boundary_size_ = max_boundary_size;
        if (scratch_size == 0)
            scratch_size = num_vertices * max_boundary_size;
        boundary_ = std::vector<int>(scratch_size, -1);
        boundary_sizes_ = std::vector<size_t>(num_vertices, 0);
        cluster_strides_ = std::vector<int>(num_vertices, -1);
    }

    inline void AddCluster(size_t cluster_id) {
        cluster_strides_[cluster_id] = num_clusters_;
        num_clusters_++;
    }

    inline bool IsEmpty() { return boundary_.empty(); }

    inline void Add(size_t cluster, size_t global_boundary_vertex_id) {
        size_t cluster_stride = cluster_strides_[cluster];
        size_t boundary_stride = boundary_sizes_[cluster_stride];
        boundary_[cluster_stride * max_boundary_size_ + boundary_stride] =
            global_boundary_vertex_id;
        boundary_sizes_[cluster_stride]++;
    }

    inline void Remove(size_t cluster, size_t local_boundary_vertex_id) {
        size_t cluster_stride = cluster_strides_[cluster];
        boundary_[cluster_stride * max_boundary_size_ +
                  local_boundary_vertex_id] = -1;
    }

    inline auto GetBoundary(size_t cluster) {
        size_t cluster_stride = cluster_strides_[cluster];
        size_t boundary_stride = boundary_sizes_[cluster_stride];
        return ClusterBoundary(boundary_, cluster_stride * max_boundary_size_,
                               cluster_stride * max_boundary_size_ +
                                   boundary_stride);
    }

    void Merge(size_t x, size_t y) {
        auto &&boundary_y = GetBoundary(y);
        for (size_t i = 0; i < boundary_y.size(); i++) {
            if (boundary_y[i] != -1) {
                Add(x, boundary_y[i]);
            }
        }
    }

    inline size_t GetSize(size_t cluster) const {
        return boundary_sizes_[cluster_strides_[cluster]];
    }

    void Defragment(size_t cluster) {
        auto &&boundary = GetBoundary(cluster);
        int next_pos = 0;
        int last_pos = -1;
        for (size_t i = 0; i < boundary.size(); i++) {
            if (boundary[i] >= 0) {
                std::swap(boundary[i], boundary[next_pos]);
                last_pos = next_pos;
                next_pos++;
            }
        }
        boundary_sizes_[cluster_strides_[cluster]] = last_pos + 1;
    }
};
}; // namespace Plaquette
