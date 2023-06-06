#pragma once

#include <iostream>
#include <random>
#include <string>
#include <tuple>

#include "DecodingGraph.hpp"
#include "Types.hpp"

namespace Plaquette {
namespace ErrorModels {

/**
 * @brief A class for generating erasure errors on a set of qubits.
 *
 */
class ErasureErrorModel {

    float
        probability_; /**< The probability of an error occurring on a qubit. */
    size_t num_qubits_; /**< The number of qubits to generate errors for. */
    std::vector<bool> erasure_; /**< A vector of bools representing if each
                                   qubit has been erased. */
    std::vector<bool>
        bit_flip_error_; /**< A vector of bools representing if each qubit has
                            experienced a bit flip error. */
    std::mt19937 generator_; /**< A Mersenne Twister engine used for generating
                                random numbers. */

  public:
    /**
     * @brief Constructor for the ErasureErrorModel class.
     *
     * @param num_qubits The number of qubits to generate errors for.
     * @param probability The probability of an error occurring on a qubit.
     * @param seed The seed used to initialize the random number generator. If
     * no seed is provided, the generator is initialized with a random seed.
     */
    ErasureErrorModel(size_t num_qubits, float probability, int seed = -1) {
        num_qubits_ = num_qubits;
        probability_ = probability;
        generator_ =
            (seed == -1)
                ? std::mt19937(time(NULL))
                : std::mt19937(seed); // Initialize engine with fixed seed
        bit_flip_error_ = std::vector<bool>(num_qubits, false);
        erasure_ = std::vector<bool>(num_qubits, false);
    }

    /**
     * @brief Generates errors for the set of qubits.
     *
     * @return A tuple containing the vectors of bools representing if each
     * qubit has experienced a bit flip error and if it has been erased.
     */
    auto GetErrors() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        for (size_t i = 0; i < num_qubits_; i++) {
            auto error = dist(generator_) < probability_;
            if (error) {
                erasure_[i] = true;
                auto bit_flip = dist(generator_) < 0.5;
                if (bit_flip) {
                    bit_flip_error_[i] = true;
                }
            }
        }

        return std::make_tuple(bit_flip_error_, erasure_);
    }
};

/**
 * @brief A class representing a bit-flip error model.
 */
class BitFlipErrorModel {

  private:
    size_t num_qubits_; /**< The number of qubits */
    float probability_; /**< The probability of a bit-flip error */
    std::vector<bool> bit_flip_error_; /**< The vector of qubits that have
                                          experienced a bit-flip error */
    std::mt19937 generator_; /**< The Mersenne Twister engine used for random
                                number generation */
    std::vector<bool> skip_vector_;

  public:
    /**
     * @brief Constructor for the BitFlipErrorModel class.
     *
     * @param num_qubits The number of qubits to model.
     * @param probability The probability of a bit-flip error.
     * @param seed Optional parameter for seeding the random number generator.
     * @param skip_vector useful for skipping erasure.
     * Default is -1.
     */
    BitFlipErrorModel(size_t num_qubits, float probability, int seed = -1,
                      const std::vector<bool> &skip_vector = {}) {
        num_qubits_ = num_qubits;
        probability_ = probability;
        generator_ = (seed == -1) ? std::mt19937(time(NULL))
                                  : std::mt19937(seed); // Initialize engine
                                                        // with fixed seed
        bit_flip_error_ = std::vector<bool>(num_qubits, false);
        skip_vector_ = skip_vector;
    }
    /**
     * @brief Generates a random bit-flip error vector.
     *
     * @return A vector of qubits that have experienced a bit-flip error.
     */
    auto GetErrors() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        for (size_t i = 0; i < num_qubits_; i++) {
            if (skip_vector_.size() == 0 || skip_vector_[i] == false) {
                bit_flip_error_[i] = dist(generator_) < probability_;
            }
        }
        return bit_flip_error_;
    }
};

}; // namespace ErrorModels
}; // namespace Plaquette
