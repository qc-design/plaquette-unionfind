import plaquette
from plaquette import syngraph
from plaquette.decoders import decoderbase
from plaquette.codes import LatticeCode
from plaquette.errors import QubitErrorsDict
from plaquette.codes import LatticeCode

import plaquette_graph as pcg
import plaquette_unionfind as puf

import numpy as np
import pymatching
import fusion_blossom as fb  # type: ignore
from typing import Optional
import time

def benchmark_code_qed_ged(code_type, size, measurement_type, p):
    assert code_type == "planar" or "rotated_planar"
    assert measurement_type == "perfect" or "imperfect"

    if code_type == "planar" and measurement_type == "imperfect":
        code = LatticeCode.make_planar(n_rounds=size-1, size=size)
    elif code_type == "planar" and measurement_type == "perfect":
        code = LatticeCode.make_planar(n_rounds=1, size=size)
    elif code_type == "rotated_planar" and measurement_type == "perfect":
        code = LatticeCode.make_rotated_planar(n_rounds=1, size=size)
    elif code_type == "rotated_planar" and measurement_type == "imperfect":
        code = LatticeCode.make_rotated_planar(n_rounds=size-1, size=size)
    else:
        raise ValueError("Invalid code_type or measurement_type")

    if measurement_type == "perfect":
        qed: QubitErrorsDict = {
            "pauli": {
                q: {"x": p / 3, "y": p / 3, "z": p / 3}
                for q in range(len(code.lattice.dataqubits))
            },
        }
    elif measurement_type == "imperfect":
        qed: QubitErrorsDict = {
            "pauli": {q: {"x": p/3, "y": p/3, "z": p/3} for q in range(len(code.lattice.dataqubits))},
            "measurement": {q.equbit_idx: {"p": p} for q in code.lattice.stabgens},
        }
    else:
        raise ValueError("Invalid measurement_type")

    return code, qed, {}


class PyMatchingComponentDecoderBenchmark(decoderbase.DecoderBackendInterface):
    """Min-weight perfect matching decoder provided by PyMatching.

    This is a low-level interface which interacts with an arbitrary graph defined
    in :class:`.SyndromeGraphComponent`. If you are working :mod:`plaquette.codes` and
    :mod:`plaquette.errors`, or if you are using :class:`.SyndromeGraph`,
    it is easier to use :class:`plaquette.decoders.interfaces.PyMatchingDecoder`
    instead of this class.
    """

    #: Reference to syndrome graph component (we retrieve the syndrome from there)
    sgraph: Optional[syngraph.SyndromeGraphComponent] = None
    #: PyMatching's matching graph
    mgraph: Optional[pymatching.Matching] = None
    #: PyMatching merge strategy for duplicate edges. Change this only if you know
    #: what you are doing.
    merge_strategy: str = "disallow"

    def set_syngraph(self, sgraph: syngraph.SyndromeGraphComponent):
        """Update syndrome graph and weights.

        This method receives the syndrome graph from the decoder interface. The received
        graph was created using the tools from the plaquette library, so this method
        rewrites the graph according to the definitions used in the PyMatching
        library. The graph and weights are inseparable within its definition,
        and when the graph is created this information must already be given.
        """
        self.sgraph = sgraph
        self.mgraph = pymatching.Matching()
        weighted = self.sgraph.is_weighted(support_mixed=False)
        if weighted:
            decoderbase.check_weights_sane(self.sgraph.edge_weights)
        boundary_idx = sgraph.n_vertices
        # Number of connections to boundary vertices for each vertex.
        boundary_edges = np.zeros([sgraph.n_vertices], dtype=int)
        for edge_idx, edge in enumerate(sgraph.edges):
            # In principle, we can omit fault_ids for stabilizer error edges. (Reason:
            # We do not at all use the information about which stab error edges were
            # selected.)
            kw = dict(fault_ids=edge_idx, merge_strategy=self.merge_strategy)
            if weighted:
                kw["weight"] = self.sgraph.edge_weights[edge_idx]
            match edge:
                case (a, b):
                    self.mgraph.add_edge(a, b, **kw)
                case (a,):
                    # Handle each dangling edge on vertex `a` by adding a connection
                    # to a *different* boundary vertex.
                    self.mgraph.add_edge(a, boundary_idx + boundary_edges[a], **kw)
                    boundary_edges[a] += 1
                case _:
                    raise ValueError("Decoder supports 2-edges and 1-edges only")
        if boundary_edges.any():
            boundary = {boundary_idx + i for i in range(boundary_edges.max())}
            self.mgraph.set_boundary_nodes(boundary)

    def update_weights(self):
        """Update weights.

        The PyMatching library receives weights and syndrome graph altogether, so one
        has to set anew the graph with the new weights and feed it to PyMatching.

        .. note::

            Although this method calls for a new set of the syndrome graph, consistency
            dictates that it should check for the existence of a graph within the class
            and create a new graph to be fed into the PyMatching library which is equal
            to the previous, but with a new variable weights.
        """
        assert self.sgraph is not None
        self.set_syngraph(self.sgraph)

    def decode(self):
        """Decode erasure and syndrome."""
        assert self.sgraph is not None
        assert self.mgraph is not None
        if self.sgraph.edge_erased.any():
            raise ValueError("Decoding erasures is not supported yet")
        start = time.time()
        result = self.mgraph.decode(self.sgraph.syndrome).astype(bool)
        end = time.time()
        print("pymatching,time:", end - start)
        self.sgraph.set_edge_decoder_results(result)



class PyMatchingDecoderBenchmark(decoderbase.DecoderInterface):
    """Min-weight perfect matching decoder provided by PyMatching.

    See :class:`.decoderbase.DecoderInterface` for supported methods.

    This is an interface to :class:`.matching.PyMatchingDecoder`.
    """

    _decoder_cls = PyMatchingComponentDecoderBenchmark

        
class FusionBlossomComponentDecoderBenchmark(decoderbase.DecoderBackendInterface):
    """Min-weight perfect matching decoder provided by Fusion Blossom.

    This is a low-level interface which interacts with an arbitrary graph defined
    in :class:`.SyndromeGraphComponent`. If you are working :mod:`plaquette.codes`
    and :mod:`plaquette.errors`, or if you are using :class:`.SyndromeGraph`, it
    is easier to use :class:`plaquette.decoders.interfaces.FusionBlossomDecoder`
    instead of this class.
    """

    #: Reference to syndrome graph component (we retrieve the syndrome from there)
    sgraph: Optional[syngraph.SyndromeGraphComponent] = None
    #: Fusion Blossom's initializer graph
    solver: Optional[fb.SolverSerial] = None

    def set_syngraph(self, sgraph: syngraph.SyndromeGraphComponent):
        """Update syndrome graph and weights.

        This method receives the syndrome graph from the decoder interface. The received
        graph was created using the tools from the library, so this method rewrites the
        graph according to the definitions used in the FusionBlossom library. The graph
        and weights are inseparable within its definition, and when the graph is created
        this information must already be given.
        """
        self.sgraph = sgraph
        if self.sgraph.is_weighted(support_mixed=False):
            decoderbase.check_weights_sane(self.sgraph.edge_weights)
            weights = (
                100 * self.sgraph.edge_weights / self.sgraph.edge_weights.max()
            ).astype(int)
        else:
            weights = np.ones(len(self.sgraph.edges)).astype(int)
        n_vertices = self.sgraph.n_vertices
        edges = []
        open_vertices: list[int] = []
        for i, edge in enumerate(self.sgraph.edges):
            weight = 2 * weights[i]
            if len(edge) == 1:
                open_vertices.append(n_vertices + len(open_vertices))
                edge = (edge[0], open_vertices[-1])
            edges.append((edge[0], edge[1], weight))
        n_vertices += len(open_vertices)
        initializer = fb.SolverInitializer(n_vertices, edges, open_vertices)
        self.solver = fb.SolverSerial(initializer)

    def update_weights(self):
        """Update weights.

        The Fusion-Blossom library receives weights and syndrome graph altogether. The
        has to be reset with the new weights and be fed it to Fusion-Blossom.

        .. note::

            Although this method calls for a new solver, consistency dictates that it
            should check for the existence of a graph within the class and create a new
            graph to be fed into the PyMatching library which is equal to the previous,
            but with a new variable weights.
        """
        assert self.sgraph is not None
        self.set_syngraph(self.sgraph)

    def get_syndrome_pattern(self):
        """Create the :class:`fb.SyndromePattern` used by the decoder.

        The solver receives the syndrome and erasure from a specific class called the
        :class:`fb.SyndromePattern`. In order to create such a class, a small
        modification must be made to the :mod:`itsqec` representation of syndromes and
        erasures.
        """
        syndrome = np.where(self.sgraph.syndrome)[0]
        erasure = np.where(self.sgraph.edge_erased)[0]
        return fb.SyndromePattern(syndrome_vertices=syndrome, erasures=erasure)

    def decode(self):
        """Decode erasure and syndrome."""
        assert self.sgraph is not None
        assert self.solver is not None
        syndrome_pattern = self.get_syndrome_pattern()
        start = time.time()
        self.solver.solve(syndrome_pattern)
        result = self.solver.subgraph(None)
        end = time.time()
        print("fusion-blossom,time:", end - start)
        result = np.array([(i in result) for i in range(len(self.sgraph.edges))])
        self.solver.clear()
        self.sgraph.set_edge_decoder_results(result)



class FusionBlossomDecoderBenchmark(decoderbase.DecoderInterface):
    """Min-weight perfect matching decoder provided by FusionBlossom.

    See :class:`.decoderbase.DecoderInterface` for supported methods.

    This is an interface to :class:`.matching.FusionBlossomDecoder`.
    """

    _decoder_cls = FusionBlossomComponentDecoderBenchmark



class UnionFindComponentDecoderBenchmark(decoderbase.DecoderBackendInterface):
    """Plaquette UnionFind decoder interface

    This is a low-level interface which interacts with an arbitrary graph defined
    in :class:`.SyndromeGraphComponent`.
    """

    sgraph: Optional[syngraph.SyndromeGraphComponent] = None
    dg: Optional[pcg.DecodingGraph] = None
    uf: Optional[puf.UnionFindDecoder] = None
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
        self.uf = puf.UnionFindDecoder(self.dg)
        
    def update_weights(self):
        assert self.sgraph_component is not None
        self.set_syngraph(self.sgraph_component)

    def decode(self):
        # we need to resize because plaquette currently doesn't add boundary points to the syndrome
        # whereas the unionfind decoder plugin does
        syndrome = np.pad(self.sgraph_component.syndrome, (0, self.boundary_length), mode='constant', constant_values=False)
        start = time.time()
        result = self.uf.decode(syndrome)
        end = time.time()
        print("plaquette-unionfind,time:", end - start)
        self.sgraph_component.set_edge_decoder_results(np.asarray(result))

class UnionFindDecoderBenchmark(decoderbase.DecoderInterface):
    """Min-weight perfect matching decoder provided by PyMatching
    See :class:`.decoderbase.DecoderInterface` for supported methods.
    This is an interface to :class:`.matching.PyMatchingMWPM`.
    """

    _decoder_cls = UnionFindComponentDecoderBenchmark
