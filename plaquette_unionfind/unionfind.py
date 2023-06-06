import numpy as np
import plaquette

from plaquette.codes import LatticeCode
from plaquette.errors import QubitErrorsDict, GateErrorsDict

# from plaquette.simulator import SimulatorSample
import plaquette.codes.latticebase as lattice
from plaquette.visualizer import LatticeVisualizer
from plaquette.codes import LatticeCode
from plaquette.syngraph import SyndromeGraph
from plaquette.decoders import decoderbase

from plaquette import syngraph
from typing import Optional
from tqdm import tqdm
import sys
import time

import plaquette_graph as pcg
from plaquette_unionfind_bindings import UnionFindDecoder
from plaquette_unionfind_bindings import PeelingDecoder


class UnionFindDecoderComponentInterface(decoderbase.DecoderBackendInterface):
    """Plaquette UnionFind decoder interface

    This is a low-level interface which interacts with an arbitrary graph defined
    in :class:`.SyndromeGraphComponent`.
    """

    sgraph: Optional[syngraph.SyndromeGraphComponent] = None
    dg: Optional[pcg.DecodingGraph] = None
    uf: Optional[UnionFindDecoder] = None
    vertex_boundary: Optional[np.ndarray] = None
    boundary_length: Optional[int] = None

    def set_syngraph(self, sgraph_component: syngraph.SyndromeGraphComponent):
        """Update syndrome graph and weights

        This method receives the syndrome graph from the decoder interface. The received
        graph was created using the tools from the itsqec library, so this method
        rewrites the graph according to the definitions used in the ItsQec C++ library
        library. The graph and weights are inseparable within its definition,
        and when the graph is created this information must already be given.
        """
        self.sgraph_component = sgraph_component

        n_vertices = sgraph_component.n_vertices
        boundary_idx = n_vertices
        edges = []
        boundary = []

        for edge_idx, edge in enumerate(sgraph_component.edges):
            match edge:
                case (a, b):
                    edges.append((a, b))
                case (a,):
                    edges.append((a, boundary_idx))
                    boundary_idx += 1
                    boundary.append(a)
                case _:
                    raise ValueError("Decoder supports 2-edges and 1-edges only")

        self.boundary_length = len(boundary)
        self.vertex_boundary = np.array([False] * n_vertices + [True] * len(boundary), dtype=bool)
        self.dg = pcg.DecodingGraph(boundary_idx, edges, self.vertex_boundary)
        self.uf = UnionFindDecoder(self.dg)

    def update_weights(self):
        assert self.sgraph_component is not None
        self.set_syngraph(self.sgraph_component)

    def decode(self):
        # we need to resize because plaquette currently doesn't add boundary points to the syndrome
        # whereas the unionfind decoder plugin does
        resized_syndrome = np.pad(
            self.sgraph_component.syndrome,
            (0, self.boundary_length),
            mode="constant",
            constant_values=False,
        )
        result = self.uf.decode(resized_syndrome, self.sgraph_component.edge_erased)
        self.sgraph_component.set_edge_decoder_results(np.asarray(result))


class UnionFindDecoderInterface(decoderbase.DecoderInterface):
    """Min-weight perfect matching decoder provided by PyMatching
    See :class:`.decoderbase.DecoderInterface` for supported methods.
    This is an interface to :class:`.matching.PyMatchingMWPM`.
    """

    _decoder_cls = UnionFindDecoderComponentInterface
