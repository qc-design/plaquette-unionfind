#include "LatticeVisualizer.hpp"
#include "StabilizerCode.hpp"
#include "ToricCode.hpp"
#include <catch2/catch.hpp>

using namespace Plaquette;

TEST_CASE("ToricCode") {

    ToricCode tc(4);

    SECTION("ToricCode()") { tc.GetNumOfQubits() == 2 * 4 * 4; }

    SECTION("ModuloCoord()") {
        auto coord = tc.ModuloCoord(-1);
        REQUIRE(coord == 7);
        coord = tc.ModuloCoord(8);
        REQUIRE(coord == 0);
        coord = tc.ModuloCoord(1);
        REQUIRE(coord == 1);
    }

    SECTION("IsPeriodic()") { REQUIRE(tc.IsPeriodic() == true); }

    SECTION("LinearizeCoord()") {
        tc.LinearizeCoord({0, 0}) == 0;
        tc.LinearizeCoord({8, 0}) == 0;
    }

    SECTION("DelinearizeCoord()") {
        auto coord = tc.DelinearizeCoord(0);
        REQUIRE(coord.first == 0);
        REQUIRE(coord.second == 0);
    }
}

TEST_CASE("StabilizerCode tests") {

    ToricCode tc(4);

    SECTION("Measure Syndrome") {

        auto lv = tc.GetVisualizer(StabilizerCode::GridType::Qubit, true, true);

        std::vector<bool> errors(tc.GetNumOfQubits(), false);
        errors[24] = true;

        auto &&syndrome =
            tc.MeasureSyndrome(errors, StabilizerCode::Stabilizer::Z);

        REQUIRE(syndrome[12] == true);
        REQUIRE(syndrome[15] == true);

        errors[25] = true;

        syndrome = tc.MeasureSyndrome(errors, StabilizerCode::Stabilizer::Z);

        REQUIRE(syndrome[13] == true);
        REQUIRE(syndrome[15] == true);

        errors[26] = true;
        errors[27] = true;

        syndrome = tc.MeasureSyndrome(errors, StabilizerCode::Stabilizer::Z);
        for (size_t i = 0; i < syndrome.size(); i++) {
            REQUIRE(syndrome[i] == false);
        }
    }

    SECTION("Measure Logical") {

        auto lv = tc.GetVisualizer(StabilizerCode::GridType::Qubit, true, true);

        std::vector<bool> errors(tc.GetNumOfQubits(), false);
        errors[24] = true;

        auto &&syndrome =
            tc.MeasureSyndrome(errors, StabilizerCode::Stabilizer::Z);
        auto meas = tc.MeasureLogical(errors, StabilizerCode::Channel::X);
        REQUIRE(meas == true);

        REQUIRE(syndrome[12] == true);
        REQUIRE(syndrome[15] == true);

        errors[25] = true;

        meas = tc.MeasureLogical(errors, StabilizerCode::Channel::X);
        REQUIRE(meas == false);

        syndrome = tc.MeasureSyndrome(errors, StabilizerCode::Stabilizer::Z);

        REQUIRE(syndrome[13] == true);
        REQUIRE(syndrome[15] == true);

        errors[26] = true;
        errors[27] = true;

        meas = tc.MeasureLogical(errors, StabilizerCode::Channel::X);
        REQUIRE(meas == false);

        syndrome = tc.MeasureSyndrome(errors, StabilizerCode::Stabilizer::Z);
        for (size_t i = 0; i < syndrome.size(); i++) {
            REQUIRE(syndrome[i] == false);
        }
    }
}
