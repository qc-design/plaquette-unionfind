#include "DecodingGraph.hpp"
#include "ErrorModels.hpp"
#include "PeelingDecoder.hpp"
#include "StabilizerCode.hpp"
#include "TestHelpers.hpp"
#include "ToricCode.hpp"
#include "UnionFindDecoder.hpp"
#include "Utils.hpp"
#include <catch2/catch.hpp>

#include <numeric>

using namespace Plaquette;
using namespace Plaquette::ErrorModels;
using namespace Plaquette::Decoders;

TEST_CASE("UnionFind PlanarCode Size=4 (p_p = 0.099, p_e = 0.0)") {

    std::vector<std::pair<size_t, size_t>> edges = {
        {0, 4},   {1, 5},   {2, 6},   {3, 7},   {4, 8},   {4, 5},   {5, 9},
        {5, 6},   {6, 10},  {6, 7},   {7, 11},  {8, 12},  {8, 9},   {9, 13},
        {9, 10},  {10, 14}, {10, 11}, {11, 15}, {12, 16}, {12, 13}, {13, 17},
        {13, 14}, {14, 18}, {14, 15}, {15, 19}};
    std::vector<std::vector<int>> qubit_to_edge = {
        {0, 1, 2, 3},     {5, 7, 9},    {4, 6, 8, 10},   {12, 14, 16},
        {11, 13, 15, 17}, {19, 21, 23}, {18, 20, 22, 24}};
    std::vector<bool> vertex_boundary = {
        true,  true,  true,  true,  false, false, false, false, false, false,
        false, false, false, false, false, false, true,  true,  true,  true};

    size_t num_vertices = vertex_boundary.size();

    DecodingGraph decoding_graph(num_vertices, edges, vertex_boundary);

    // These cases have been checked by hand
    SECTION("Seed #0") {
        std::vector<bool> pauli_errors = {
            false, false, false, false, false, false, false, false, true,
            false, false, true,  false, false, false, false, false, false,
            false, false, true,  false, false, false, false};
        std::vector<bool> syndrome = {false, false, false, false, false,
                                      false, true,  false, true,  false,
                                      true,  false, true,  true,  false,
                                      false, false, false, false, false};
        std::vector<bool> modified_erasure_check = {
            false, false, false, false, true,  false, false, false, true,
            false, false, true,  true,  true,  false, false, false, false,
            true,  true,  true,  true,  false, false, false};

        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome);
        uf_decoder.SyndromeValidation();
        auto modified_erasure = uf_decoder.GetModifiedErasure();
        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }
    SECTION("Seed #20") {
        std::vector<bool> pauli_errors = {
            true,  true,  false, false, false, false, false, false, false,
            false, false, false, true,  false, false, false, true,  false,
            false, false, false, false, true,  false, false};
        std::vector<bool> syndrome = {false, false, false, false, true,
                                      true,  false, false, true,  true,
                                      true,  true,  false, false, true,
                                      false, false, false, false, false};
        std::vector<bool> modified_erasure_check = {
            true,  true,  false, false, true, true, true, true, true,
            false, true,  true,  true,  true, true, true, true, true,
            false, false, false, true,  true, true, false};

        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome);
        uf_decoder.SyndromeValidation();
        auto modified_erasure = uf_decoder.GetModifiedErasure();
        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }
    SECTION("Seed #191") {
        std::vector<bool> pauli_errors = {
            false, false, false, false, false, false, false, true,  false,
            false, false, false, false, false, false, true,  false, false,
            false, false, false, false, false, false, true};
        std::vector<bool> syndrome = {false, false, false, false, false,
                                      true,  true,  false, false, false,
                                      true,  false, false, false, true,
                                      true,  false, false, false, false};

        std::vector<bool> modified_erasure_check = {
            false, true,  true,  false, false, true, true, true, true,
            true,  false, false, false, false, true, true, true, true,
            false, false, false, true,  true,  true, true};

        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome);
        uf_decoder.SyndromeValidation();
        auto modified_erasure = uf_decoder.GetModifiedErasure();
        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }
}

TEST_CASE("UnionFind ToricCode Size=4 (p_p = 0.099, p_e = 0.0)") {

    size_t lattice_size = 4;
    size_t num_vertices = lattice_size * lattice_size;
    std::vector<std::pair<size_t, size_t>> edges = {
        {0, 3},   {0, 1},   {0, 12},  {0, 4},  {1, 2},   {1, 13},  {1, 5},
        {2, 3},   {2, 14},  {2, 6},   {3, 15}, {3, 7},   {4, 7},   {4, 5},
        {4, 8},   {5, 6},   {5, 9},   {6, 7},  {6, 10},  {7, 11},  {8, 11},
        {8, 9},   {8, 12},  {9, 10},  {9, 13}, {10, 11}, {10, 14}, {11, 15},
        {12, 15}, {12, 13}, {13, 14}, {14, 15}};
    std::vector<std::vector<int>> qubit_to_edge = {
        {2, 5, 8, 10},    {0, 1, 4, 7},     {3, 6, 9, 11},    {12, 13, 15, 17},
        {14, 16, 18, 19}, {20, 21, 23, 25}, {22, 24, 26, 27}, {28, 29, 30, 31}};

    std::vector<bool> vertex_boundary(num_vertices, false);

    DecodingGraph decoding_graph(num_vertices, edges, vertex_boundary);

    SECTION("Seed #0") {
        std::vector<bool> pauli_errors = {
            false, false, false, false, false, false, false, false,
            true,  false, false, true,  false, false, false, false,
            false, false, false, false, true,  false, false, false,
            false, false, false, true,  false, false, false, false};
        std::vector<bool> syndrome = {false, false, true, true,  false, false,
                                      false, true,  true, false, false, false,
                                      false, false, true, true};

        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome);
        uf_decoder.SyndromeValidation();
        auto modified_erasure = uf_decoder.GetModifiedErasure();

        std::vector<bool> modified_erasure_check = {
            false, false, false, false, false, false, false, true,
            true,  false, true,  true,  true,  false, true,  false,
            false, false, false, true,  true,  true,  true,  false,
            false, false, false, true,  true,  false, false, true};

        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }

    SECTION("Seed #24") {
        std::vector<bool> pauli_errors = {
            false, false, false, false, true,  false, false, false,
            false, true,  false, false, true,  false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false};
        std::vector<bool> syndrome = {false, true,  false, false, true,  false,
                                      true,  true,  false, false, false, false,
                                      false, false, false, false};
        std::vector<bool> modified_erasure_check = {
            false, true,  false, true,  true,  true,  true,  false,
            false, true,  false, true,  true,  true,  true,  true,
            false, true,  true,  true,  false, false, false, false,
            false, false, false, false, false, false, false, false};

        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome);
        uf_decoder.SyndromeValidation();
        auto modified_erasure = uf_decoder.GetModifiedErasure();

        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }
    SECTION("Seed #602") {
        std::vector<bool> pauli_errors = {
            false, false, false, true,  false, false, true,  false,
            false, false, false, false, true,  false, false, false,
            false, false, false, false, false, false, false, false,
            false, true,  false, false, false, true,  false, false};
        std::vector<bool> syndrome = {true,  true, false, false, false, true,
                                      false, true, false, false, true,  true,
                                      true,  true, false, false};
        std::vector<bool> modified_erasure_check = {
            true,  true,  true,  true, true, true,  true,  false,
            false, false, false, true, true, true,  false, true,
            true,  true,  true,  true, true, false, true,  true,
            true,  true,  true,  true, true, true,  true,  false};

        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome);
        uf_decoder.SyndromeValidation();
        auto modified_erasure = uf_decoder.GetModifiedErasure();

        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }
}

TEST_CASE("UnionFind ToricCode Size=5") {

    size_t num_trials = 1000;
    size_t lattice_size = 5;
    size_t num_vertices = lattice_size * lattice_size;
    size_t num_qubits = 2 * lattice_size * lattice_size;

    std::vector<std::pair<size_t, size_t>> edges = {
        {0, 4},   {0, 1},   {0, 20},  {0, 5},   {1, 2},   {1, 21},  {1, 6},
        {2, 3},   {2, 22},  {2, 7},   {3, 4},   {3, 23},  {3, 8},   {4, 24},
        {4, 9},   {5, 9},   {5, 6},   {5, 10},  {6, 7},   {6, 11},  {7, 8},
        {7, 12},  {8, 9},   {8, 13},  {9, 14},  {10, 14}, {10, 11}, {10, 15},
        {11, 12}, {11, 16}, {12, 13}, {12, 17}, {13, 14}, {13, 18}, {14, 19},
        {15, 19}, {15, 16}, {15, 20}, {16, 17}, {16, 21}, {17, 18}, {17, 22},
        {18, 19}, {18, 23}, {19, 24}, {20, 24}, {20, 21}, {21, 22}, {22, 23},
        {23, 24}};
    std::vector<std::vector<int>> qubit_to_edge = {
        {2, 5, 8, 11, 13},    {0, 1, 4, 7, 10},     {3, 6, 9, 12, 14},
        {15, 16, 18, 20, 22}, {17, 19, 21, 23, 24}, {25, 26, 28, 30, 32},
        {27, 29, 31, 33, 34}, {35, 36, 38, 40, 42}, {37, 39, 41, 43, 44},
        {45, 46, 47, 48, 49}};
    DecodingGraph decoding_graph(num_vertices, edges,
                                 std::vector<bool>(num_vertices, false));

    SECTION("Check Peeled Syndrome") {
        for (size_t i = 0; i < num_trials; i++) {
            BitFlipErrorModel error_model(num_qubits, 0.099, 12344 + 2000 * i);
            const auto &error = error_model.GetErrors();
            std::vector<bool> syndrome = MeasureSyndrome(decoding_graph, error);

            Decoders::UnionFindDecoder uf_decoder(decoding_graph);
            const auto &correction = uf_decoder.Decode(syndrome);
            auto uncorrected_errors = Utils::SetXor(error, correction);
            const auto &uncorrected_syndrome =
                MeasureSyndrome(decoding_graph, uncorrected_errors);
            auto sum = std::accumulate(uncorrected_syndrome.begin(),
                                       uncorrected_syndrome.end(), 0);
            REQUIRE(sum == 0);
        }
    }
}

TEST_CASE("UnionFind ToricCode Class Size=5") {

    size_t num_trials = 1000;
    size_t lattice_size = 5;
    size_t num_vertices = lattice_size * lattice_size;
    size_t num_qubits = 2 * lattice_size * lattice_size;
    ToricCode tc(5);
    const auto &decoding_graph = tc.GetZStabilizerDecodingGraph();

    SECTION("Check Peeled Syndrome") {
        for (size_t i = 0; i < num_trials; i++) {
            BitFlipErrorModel error_model(num_qubits, 0.099, 12344 + 2000 * i);
            const auto &error = error_model.GetErrors();
            std::vector<bool> syndrome = MeasureSyndrome(decoding_graph, error);

            Decoders::UnionFindDecoder uf_decoder(decoding_graph);
            const auto &correction = uf_decoder.Decode(syndrome);
            auto uncorrected_errors = Utils::SetXor(error, correction);
            const auto &uncorrected_syndrome =
                MeasureSyndrome(decoding_graph, uncorrected_errors);
            auto sum = std::accumulate(uncorrected_syndrome.begin(),
                                       uncorrected_syndrome.end(), 0);
            REQUIRE(sum == 0);
        }
    }
}

TEST_CASE("UnionFind ToricCode Class With Erasure Size=5") {

    size_t num_trials = 1000;
    size_t lattice_size = 5;
    size_t num_vertices = lattice_size * lattice_size;
    size_t num_qubits = 2 * lattice_size * lattice_size;
    ToricCode tc(5);
    const auto &decoding_graph = tc.GetZStabilizerDecodingGraph();

    SECTION("Check Peeled Syndrome") {
        for (size_t i = 0; i < num_trials; i++) {
            ErasureErrorModel erasure_model(num_qubits, 0.1, 33344 + 3000 * i);
            const auto &[erasure_bit_flip_error, erasure] =
                erasure_model.GetErrors();
            BitFlipErrorModel bitflip_model(num_qubits, 0.1, 12344 + 2000 * i,
                                            erasure);

            const auto &bit_flip_error = bitflip_model.GetErrors();
            auto combined_bit_flip_error =
                Utils::SetXor(bit_flip_error, erasure_bit_flip_error);
            std::vector<bool> syndrome = tc.MeasureSyndrome(
                combined_bit_flip_error, StabilizerCode::Stabilizer::Z);

            Decoders::UnionFindDecoder uf_decoder_0(decoding_graph);
            uf_decoder_0.SetSyndromeAndErasure(syndrome, erasure);
            auto modified_erasure = uf_decoder_0.GetModifiedErasure();

            for (size_t e = 0; e < modified_erasure.size(); e++) {
                REQUIRE(erasure[e] == modified_erasure[e]);
            }

            syndrome = tc.MeasureSyndrome(combined_bit_flip_error,
                                          StabilizerCode::Stabilizer::Z);

            Decoders::UnionFindDecoder uf_decoder(decoding_graph);
            auto correction = uf_decoder.Decode(syndrome, erasure);
            auto uncorrected_errors =
                Utils::SetXor(combined_bit_flip_error, correction);
            const auto &uncorrected_syndrome =
                MeasureSyndrome(decoding_graph, uncorrected_errors);
            auto sum = std::accumulate(uncorrected_syndrome.begin(),
                                       uncorrected_syndrome.end(), 0);
            REQUIRE(sum == 0);
        }
    }
}

TEST_CASE(
    "PlanarCode n_rounds=1 measurement error, depolarization error p = 0.08") {
    size_t num_vertices = 30;
    std::vector<std::pair<size_t, size_t>> edges{
        std::make_pair<size_t, size_t>(0, 20),
        std::make_pair<size_t, size_t>(0, 1),
        std::make_pair<size_t, size_t>(1, 2),
        std::make_pair<size_t, size_t>(2, 3),
        std::make_pair<size_t, size_t>(3, 21),
        std::make_pair<size_t, size_t>(0, 4),
        std::make_pair<size_t, size_t>(1, 5),
        std::make_pair<size_t, size_t>(2, 6),
        std::make_pair<size_t, size_t>(3, 7),
        std::make_pair<size_t, size_t>(4, 22),
        std::make_pair<size_t, size_t>(4, 5),
        std::make_pair<size_t, size_t>(5, 6),
        std::make_pair<size_t, size_t>(6, 7),
        std::make_pair<size_t, size_t>(7, 23),
        std::make_pair<size_t, size_t>(4, 8),
        std::make_pair<size_t, size_t>(5, 9),
        std::make_pair<size_t, size_t>(6, 10),
        std::make_pair<size_t, size_t>(7, 11),
        std::make_pair<size_t, size_t>(8, 24),
        std::make_pair<size_t, size_t>(8, 9),
        std::make_pair<size_t, size_t>(9, 10),
        std::make_pair<size_t, size_t>(10, 11),
        std::make_pair<size_t, size_t>(11, 25),
        std::make_pair<size_t, size_t>(8, 12),
        std::make_pair<size_t, size_t>(9, 13),
        std::make_pair<size_t, size_t>(10, 14),
        std::make_pair<size_t, size_t>(11, 15),
        std::make_pair<size_t, size_t>(12, 26),
        std::make_pair<size_t, size_t>(12, 13),
        std::make_pair<size_t, size_t>(13, 14),
        std::make_pair<size_t, size_t>(14, 15),
        std::make_pair<size_t, size_t>(15, 27),
        std::make_pair<size_t, size_t>(12, 16),
        std::make_pair<size_t, size_t>(13, 17),
        std::make_pair<size_t, size_t>(14, 18),
        std::make_pair<size_t, size_t>(15, 19),
        std::make_pair<size_t, size_t>(16, 28),
        std::make_pair<size_t, size_t>(16, 17),
        std::make_pair<size_t, size_t>(17, 18),
        std::make_pair<size_t, size_t>(18, 19),
        std::make_pair<size_t, size_t>(19, 29)};

    std::vector<bool> vertex_boundary{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    std::vector<bool> syndrome_component_0{0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<bool> modified_erasure_check{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    DecodingGraph decoding_graph(num_vertices, edges, vertex_boundary);
    Decoders::UnionFindDecoder uf_decoder(decoding_graph);
    uf_decoder.SetSyndrome(syndrome_component_0);
    uf_decoder.SyndromeValidation();

    auto modified_erasure = uf_decoder.GetModifiedErasure();
    for (auto i = 0; i < modified_erasure.size(); i++) {
        REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
    }
}

TEST_CASE(
    "PlanarCode n_rounds=4 measurement error, depolarization error p = 0.08") {
    size_t num_vertices = 120;
    std::vector<std::pair<size_t, size_t>> edges{
        std::make_pair<size_t, size_t>(0, 80),
        std::make_pair<size_t, size_t>(0, 1),
        std::make_pair<size_t, size_t>(1, 2),
        std::make_pair<size_t, size_t>(2, 3),
        std::make_pair<size_t, size_t>(3, 81),
        std::make_pair<size_t, size_t>(0, 4),
        std::make_pair<size_t, size_t>(1, 5),
        std::make_pair<size_t, size_t>(2, 6),
        std::make_pair<size_t, size_t>(3, 7),
        std::make_pair<size_t, size_t>(4, 82),
        std::make_pair<size_t, size_t>(4, 5),
        std::make_pair<size_t, size_t>(5, 6),
        std::make_pair<size_t, size_t>(6, 7),
        std::make_pair<size_t, size_t>(7, 83),
        std::make_pair<size_t, size_t>(4, 8),
        std::make_pair<size_t, size_t>(5, 9),
        std::make_pair<size_t, size_t>(6, 10),
        std::make_pair<size_t, size_t>(7, 11),
        std::make_pair<size_t, size_t>(8, 84),
        std::make_pair<size_t, size_t>(8, 9),
        std::make_pair<size_t, size_t>(9, 10),
        std::make_pair<size_t, size_t>(10, 11),
        std::make_pair<size_t, size_t>(11, 85),
        std::make_pair<size_t, size_t>(8, 12),
        std::make_pair<size_t, size_t>(9, 13),
        std::make_pair<size_t, size_t>(10, 14),
        std::make_pair<size_t, size_t>(11, 15),
        std::make_pair<size_t, size_t>(12, 86),
        std::make_pair<size_t, size_t>(12, 13),
        std::make_pair<size_t, size_t>(13, 14),
        std::make_pair<size_t, size_t>(14, 15),
        std::make_pair<size_t, size_t>(15, 87),
        std::make_pair<size_t, size_t>(12, 16),
        std::make_pair<size_t, size_t>(13, 17),
        std::make_pair<size_t, size_t>(14, 18),
        std::make_pair<size_t, size_t>(15, 19),
        std::make_pair<size_t, size_t>(16, 88),
        std::make_pair<size_t, size_t>(16, 17),
        std::make_pair<size_t, size_t>(17, 18),
        std::make_pair<size_t, size_t>(18, 19),
        std::make_pair<size_t, size_t>(19, 89),
        std::make_pair<size_t, size_t>(20, 90),
        std::make_pair<size_t, size_t>(20, 21),
        std::make_pair<size_t, size_t>(21, 22),
        std::make_pair<size_t, size_t>(22, 23),
        std::make_pair<size_t, size_t>(23, 91),
        std::make_pair<size_t, size_t>(20, 24),
        std::make_pair<size_t, size_t>(21, 25),
        std::make_pair<size_t, size_t>(22, 26),
        std::make_pair<size_t, size_t>(23, 27),
        std::make_pair<size_t, size_t>(24, 92),
        std::make_pair<size_t, size_t>(24, 25),
        std::make_pair<size_t, size_t>(25, 26),
        std::make_pair<size_t, size_t>(26, 27),
        std::make_pair<size_t, size_t>(27, 93),
        std::make_pair<size_t, size_t>(24, 28),
        std::make_pair<size_t, size_t>(25, 29),
        std::make_pair<size_t, size_t>(26, 30),
        std::make_pair<size_t, size_t>(27, 31),
        std::make_pair<size_t, size_t>(28, 94),
        std::make_pair<size_t, size_t>(28, 29),
        std::make_pair<size_t, size_t>(29, 30),
        std::make_pair<size_t, size_t>(30, 31),
        std::make_pair<size_t, size_t>(31, 95),
        std::make_pair<size_t, size_t>(28, 32),
        std::make_pair<size_t, size_t>(29, 33),
        std::make_pair<size_t, size_t>(30, 34),
        std::make_pair<size_t, size_t>(31, 35),
        std::make_pair<size_t, size_t>(32, 96),
        std::make_pair<size_t, size_t>(32, 33),
        std::make_pair<size_t, size_t>(33, 34),
        std::make_pair<size_t, size_t>(34, 35),
        std::make_pair<size_t, size_t>(35, 97),
        std::make_pair<size_t, size_t>(32, 36),
        std::make_pair<size_t, size_t>(33, 37),
        std::make_pair<size_t, size_t>(34, 38),
        std::make_pair<size_t, size_t>(35, 39),
        std::make_pair<size_t, size_t>(36, 98),
        std::make_pair<size_t, size_t>(36, 37),
        std::make_pair<size_t, size_t>(37, 38),
        std::make_pair<size_t, size_t>(38, 39),
        std::make_pair<size_t, size_t>(39, 99),
        std::make_pair<size_t, size_t>(40, 100),
        std::make_pair<size_t, size_t>(40, 41),
        std::make_pair<size_t, size_t>(41, 42),
        std::make_pair<size_t, size_t>(42, 43),
        std::make_pair<size_t, size_t>(43, 101),
        std::make_pair<size_t, size_t>(40, 44),
        std::make_pair<size_t, size_t>(41, 45),
        std::make_pair<size_t, size_t>(42, 46),
        std::make_pair<size_t, size_t>(43, 47),
        std::make_pair<size_t, size_t>(44, 102),
        std::make_pair<size_t, size_t>(44, 45),
        std::make_pair<size_t, size_t>(45, 46),
        std::make_pair<size_t, size_t>(46, 47),
        std::make_pair<size_t, size_t>(47, 103),
        std::make_pair<size_t, size_t>(44, 48),
        std::make_pair<size_t, size_t>(45, 49),
        std::make_pair<size_t, size_t>(46, 50),
        std::make_pair<size_t, size_t>(47, 51),
        std::make_pair<size_t, size_t>(48, 104),
        std::make_pair<size_t, size_t>(48, 49),
        std::make_pair<size_t, size_t>(49, 50),
        std::make_pair<size_t, size_t>(50, 51),
        std::make_pair<size_t, size_t>(51, 105),
        std::make_pair<size_t, size_t>(48, 52),
        std::make_pair<size_t, size_t>(49, 53),
        std::make_pair<size_t, size_t>(50, 54),
        std::make_pair<size_t, size_t>(51, 55),
        std::make_pair<size_t, size_t>(52, 106),
        std::make_pair<size_t, size_t>(52, 53),
        std::make_pair<size_t, size_t>(53, 54),
        std::make_pair<size_t, size_t>(54, 55),
        std::make_pair<size_t, size_t>(55, 107),
        std::make_pair<size_t, size_t>(52, 56),
        std::make_pair<size_t, size_t>(53, 57),
        std::make_pair<size_t, size_t>(54, 58),
        std::make_pair<size_t, size_t>(55, 59),
        std::make_pair<size_t, size_t>(56, 108),
        std::make_pair<size_t, size_t>(56, 57),
        std::make_pair<size_t, size_t>(57, 58),
        std::make_pair<size_t, size_t>(58, 59),
        std::make_pair<size_t, size_t>(59, 109),
        std::make_pair<size_t, size_t>(60, 110),
        std::make_pair<size_t, size_t>(60, 61),
        std::make_pair<size_t, size_t>(61, 62),
        std::make_pair<size_t, size_t>(62, 63),
        std::make_pair<size_t, size_t>(63, 111),
        std::make_pair<size_t, size_t>(60, 64),
        std::make_pair<size_t, size_t>(61, 65),
        std::make_pair<size_t, size_t>(62, 66),
        std::make_pair<size_t, size_t>(63, 67),
        std::make_pair<size_t, size_t>(64, 112),
        std::make_pair<size_t, size_t>(64, 65),
        std::make_pair<size_t, size_t>(65, 66),
        std::make_pair<size_t, size_t>(66, 67),
        std::make_pair<size_t, size_t>(67, 113),
        std::make_pair<size_t, size_t>(64, 68),
        std::make_pair<size_t, size_t>(65, 69),
        std::make_pair<size_t, size_t>(66, 70),
        std::make_pair<size_t, size_t>(67, 71),
        std::make_pair<size_t, size_t>(68, 114),
        std::make_pair<size_t, size_t>(68, 69),
        std::make_pair<size_t, size_t>(69, 70),
        std::make_pair<size_t, size_t>(70, 71),
        std::make_pair<size_t, size_t>(71, 115),
        std::make_pair<size_t, size_t>(68, 72),
        std::make_pair<size_t, size_t>(69, 73),
        std::make_pair<size_t, size_t>(70, 74),
        std::make_pair<size_t, size_t>(71, 75),
        std::make_pair<size_t, size_t>(72, 116),
        std::make_pair<size_t, size_t>(72, 73),
        std::make_pair<size_t, size_t>(73, 74),
        std::make_pair<size_t, size_t>(74, 75),
        std::make_pair<size_t, size_t>(75, 117),
        std::make_pair<size_t, size_t>(72, 76),
        std::make_pair<size_t, size_t>(73, 77),
        std::make_pair<size_t, size_t>(74, 78),
        std::make_pair<size_t, size_t>(75, 79),
        std::make_pair<size_t, size_t>(76, 118),
        std::make_pair<size_t, size_t>(76, 77),
        std::make_pair<size_t, size_t>(77, 78),
        std::make_pair<size_t, size_t>(78, 79),
        std::make_pair<size_t, size_t>(79, 119),
        std::make_pair<size_t, size_t>(0, 20),
        std::make_pair<size_t, size_t>(1, 21),
        std::make_pair<size_t, size_t>(2, 22),
        std::make_pair<size_t, size_t>(3, 23),
        std::make_pair<size_t, size_t>(4, 24),
        std::make_pair<size_t, size_t>(5, 25),
        std::make_pair<size_t, size_t>(6, 26),
        std::make_pair<size_t, size_t>(7, 27),
        std::make_pair<size_t, size_t>(8, 28),
        std::make_pair<size_t, size_t>(9, 29),
        std::make_pair<size_t, size_t>(10, 30),
        std::make_pair<size_t, size_t>(11, 31),
        std::make_pair<size_t, size_t>(12, 32),
        std::make_pair<size_t, size_t>(13, 33),
        std::make_pair<size_t, size_t>(14, 34),
        std::make_pair<size_t, size_t>(15, 35),
        std::make_pair<size_t, size_t>(16, 36),
        std::make_pair<size_t, size_t>(17, 37),
        std::make_pair<size_t, size_t>(18, 38),
        std::make_pair<size_t, size_t>(19, 39),
        std::make_pair<size_t, size_t>(20, 40),
        std::make_pair<size_t, size_t>(21, 41),
        std::make_pair<size_t, size_t>(22, 42),
        std::make_pair<size_t, size_t>(23, 43),
        std::make_pair<size_t, size_t>(24, 44),
        std::make_pair<size_t, size_t>(25, 45),
        std::make_pair<size_t, size_t>(26, 46),
        std::make_pair<size_t, size_t>(27, 47),
        std::make_pair<size_t, size_t>(28, 48),
        std::make_pair<size_t, size_t>(29, 49),
        std::make_pair<size_t, size_t>(30, 50),
        std::make_pair<size_t, size_t>(31, 51),
        std::make_pair<size_t, size_t>(32, 52),
        std::make_pair<size_t, size_t>(33, 53),
        std::make_pair<size_t, size_t>(34, 54),
        std::make_pair<size_t, size_t>(35, 55),
        std::make_pair<size_t, size_t>(36, 56),
        std::make_pair<size_t, size_t>(37, 57),
        std::make_pair<size_t, size_t>(38, 58),
        std::make_pair<size_t, size_t>(39, 59),
        std::make_pair<size_t, size_t>(40, 60),
        std::make_pair<size_t, size_t>(41, 61),
        std::make_pair<size_t, size_t>(42, 62),
        std::make_pair<size_t, size_t>(43, 63),
        std::make_pair<size_t, size_t>(44, 64),
        std::make_pair<size_t, size_t>(45, 65),
        std::make_pair<size_t, size_t>(46, 66),
        std::make_pair<size_t, size_t>(47, 67),
        std::make_pair<size_t, size_t>(48, 68),
        std::make_pair<size_t, size_t>(49, 69),
        std::make_pair<size_t, size_t>(50, 70),
        std::make_pair<size_t, size_t>(51, 71),
        std::make_pair<size_t, size_t>(52, 72),
        std::make_pair<size_t, size_t>(53, 73),
        std::make_pair<size_t, size_t>(54, 74),
        std::make_pair<size_t, size_t>(55, 75),
        std::make_pair<size_t, size_t>(56, 76),
        std::make_pair<size_t, size_t>(57, 77),
        std::make_pair<size_t, size_t>(58, 78),
        std::make_pair<size_t, size_t>(59, 79)};

    std::vector<bool> vertex_boundary{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    std::vector<bool> syndrome_component_0{
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<bool> modified_erasure_check{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
    DecodingGraph decoding_graph(num_vertices, edges, vertex_boundary);

    SECTION("Check Modified Erasure") {
        Decoders::UnionFindDecoder uf_decoder(decoding_graph);
        uf_decoder.SetSyndrome(syndrome_component_0);
        uf_decoder.SyndromeValidation();

        auto modified_erasure = uf_decoder.GetModifiedErasure();
        for (auto i = 0; i < modified_erasure.size(); i++) {
            REQUIRE(modified_erasure[i] == modified_erasure_check[i]);
        }
    }
}
