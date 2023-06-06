#include <functional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "DecodingGraph.hpp"
#include "PeelingDecoder.hpp"
#include "Types.hpp"
#include "UnionFindDecoder.hpp"

namespace {
using namespace Plaquette;
using namespace Plaquette::Decoders;
using namespace Plaquette::Types;
namespace py = pybind11;

PYBIND11_MODULE(plaquette_unionfind_bindings, m) {

    pybind11::class_<PeelingDecoder>(m, "PeelingDecoder")
        .def(pybind11::init<>())
        .def("decode", &PeelingDecoder::Decode);

    pybind11::class_<UnionFindDecoder>(m, "UnionFindDecoder")
        .def(pybind11::init<DecodingGraph>())
        .def(pybind11::init<DecodingGraph, const std::vector<float> &, float>())
        .def("decode",
             py::overload_cast<std::vector<bool> &>(&UnionFindDecoder::Decode),
             "Decode syndrome")
        .def("decode",
             py::overload_cast<std::vector<bool> &, const std::vector<bool> &>(
                 &UnionFindDecoder::Decode),
             "Decode syndrome with erasure")
        .def("get_modified_erasure", &UnionFindDecoder::GetModifiedErasure);
}
} // namespace
