##################################
Plaquette UnionFind Decoder Plugin
##################################

.. image:: https://github.com/qc-design/plaquette-unionfind/actions/workflows/tests_linux.yml/badge.svg
    :target: https://github.com/qc-design/plaquette-unionfind/actions/workflows/tests_linux.yml

.. image:: https://github.com/qc-design/plaquette-unionfind/actions/workflows/tests_windows.yml/badge.svg
    :target: https://github.com/qc-design/plaquette-unionfind/actions/workflows/tests_windows.yml

.. header-start-inclusion-marker-do-not-remove

.. contents:: Table of Contents

.. about-start-inclusion-marker-do-not-remove

About 
=====

The `Plaquette-UnionFind <https://github.com/qc-design/plaquette-unionfind>`_ plugin
extends the `Plaquette <https://github.com/qc-design/plaquette>`_ error correction
software, providing a fast unionfind decoder written in C++. See the `Benchmarks`_
section for a comparison to other known decoding packages.

.. about-end-inclusion-marker-do-not-remove

.. installation-start-inclusion-marker-do-not-remove

Installation
============

The basic dependencies for installation are cmake and ninja.

The C++ tests/examples and python bindings can be built independently by

.. code-block:: console

   cmake -Bbuild -G Ninja -DPLAQUETTE_UNIONFIND_BUILD_TESTS=On -DPLAQUETTE_UNIONFIND_BUILD_BINDINGS=On
   cmake --build ./build


You can run the C++ backend tests with

.. code-block:: console

   make test-cpp


You can install just the python interface with (this quietly builds the C++ backend):

.. code-block:: console

   pip install -r requirements.txt
   pip install .

or get the latest stable release from PyPI

.. code-block:: console

    pip install plaquette_unionfind


You can run the python frontend tests with

.. code-block:: console

   make test-python

.. installation-end-inclusion-marker-do-not-remove

.. benchmark-start-inclusion-marker-do-not-remove

Usage
==========

Python Frontend
---------------

.. code-block:: python

    import plaquette_unionfind as puf
    import plaquette_graph as pg

    vertex_boundary = [False] * 12 + [True] * 8
    edges = [(0, 12), (0, 1), (1, 2), (2, 13), (0, 3), (1, 4), (2, 5), (3, 14), (3, 4),
             (4, 5), (5, 15), (3, 6), (4, 7), (5, 8), (6, 16), (6, 7), (7, 8), (8, 17),
             (6, 9), (7, 10), (8, 11), (9, 18), (9, 10), (10, 11), (11, 19)]
    syndrome = [False, False, True, True, False, True, True, False, True, False, True,
                False, False, False, False, False, False, False, False, False]
    dg = pg.DecodingGraph(num_vertices, edges, vertex_boundary)
    uf = puf.UnionFindDecoder(dg)
    correction = uf.decode(syndrome)


C++ Backend
-----------

.. code-block:: cpp

    #include "DecodingGraph.hpp"
    #include "UnionFindDecoder.hpp"

    int main(int argc, char *argv[]) {

        using namespace Plaquette;
        //a vector storing a flag that is 1 if the vertex is on the boundary
        std::vector<bool> vertex_boundary = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
        std::vector<std::pair<size_t, size_t>> edges = {
            {0, 12}, {0, 1},  {1, 2},   {2, 13}, {0, 3}, {1, 4},  {2, 5},
            {3, 14}, {3, 4},  {4, 5},   {5, 15}, {3, 6}, {4, 7},  {5, 8},
            {6, 16}, {6, 7},  {7, 8},   {8, 17}, {6, 9}, {7, 10}, {8, 11},
            {9, 18}, {9, 10}, {10, 11}, {11, 19}};

        std::vector<bool> syndrome = {false, false, true,  true,  false, true,  true,
                                      false, true,  false, true,  false, false, false,
                                      false, false, false, false, false, false};

        auto decoding_graph = DecodingGraph(vertex_boundary.size(),
                                          edges, vertex_boundary);
        UnionFindDecoder decoder(decoding_graph);
        auto correction = decoder.Decode(syndrome);
    }

Interface to Plaquette
----------------------

`Plaquette <https://github.com/qc-design/plaquette>`_ is undergoing heavy development, so this interface is likely to change. If you are benchmarking
our decoder, please do not use the plaquette interface unless you know what you are doing. You will be timing other computations unrelated to the decoding.

.. code-block:: python

    from plaquette.codes import LatticeCode
    import plaquette_unionfind
    
        code = LatticeCode.make_planar(n_rounds=1, size=size)
        qed = {
            "pauli": {q.equbit_idx: {"x": error_rate} for q in code.lattice.dataqubits}
        }
        decoder = plaquette_unionfind.UnionFindDecoderInterface.from_code(code, qed, weighted=False)
    
Documentation
=============

To generate the documentation you will need to install graphviz and doxygen. Then run

.. code-block:: console

   pip install -r doc/requirements.txt
   make docs
   firefox ./doc/_build/html/index.html

Here is a live link to the documentation: https://docs.plaquette.design/projects/unionfind
