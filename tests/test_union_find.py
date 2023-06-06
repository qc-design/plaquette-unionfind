import pytest
import plaquette_unionfind as pcu
import plaquette_graph as pcg


class TestToricNoErasureSize4:
    @pytest.fixture
    def get_decoding_graph(self):
        lattice_size = 4
        num_vertices = lattice_size * lattice_size
        edges = [
            (0, 3),
            (0, 1),
            (0, 12),
            (0, 4),
            (1, 2),
            (1, 13),
            (1, 5),
            (2, 3),
            (2, 14),
            (2, 6),
            (3, 15),
            (3, 7),
            (4, 7),
            (4, 5),
            (4, 8),
            (5, 6),
            (5, 9),
            (6, 7),
            (6, 10),
            (7, 11),
            (8, 11),
            (8, 9),
            (8, 12),
            (9, 10),
            (9, 13),
            (10, 11),
            (10, 14),
            (11, 15),
            (12, 15),
            (12, 13),
            (13, 14),
            (14, 15),
        ]
        vertex_boundary = [False] * num_vertices
        dg = pcg.DecodingGraph(num_vertices, edges, vertex_boundary)
        return num_vertices, dg

    def test_seed_0(self, get_decoding_graph):
        syndrome = [
            False,
            False,
            True,
            True,
            False,
            False,
            False,
            True,
            True,
            False,
            False,
            False,
            False,
            False,
            True,
            True,
        ]

        modified_erasure_check = [
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            True,
            True,
            False,
            True,
            True,
            True,
            False,
            True,
            False,
            False,
            False,
            False,
            True,
            True,
            True,
            True,
            False,
            False,
            False,
            False,
            True,
            True,
            False,
            False,
            True,
        ]
        num_vertices, dg = get_decoding_graph
        uf = pcu.UnionFindDecoder(dg)
        uf.decode(syndrome)
        mf = uf.get_modified_erasure()

        for i in range(num_vertices):
            assert mf[i] == modified_erasure_check[i]


class TestPlanarNoErasureSize4:
    @pytest.fixture
    def get_decoding_graph(self):
        lattice_size = 4
        edges = [
            (0, 4),
            (1, 5),
            (2, 6),
            (3, 7),
            (4, 8),
            (4, 5),
            (5, 9),
            (5, 6),
            (6, 10),
            (6, 7),
            (7, 11),
            (8, 12),
            (8, 9),
            (9, 13),
            (9, 10),
            (10, 14),
            (10, 11),
            (11, 15),
            (12, 16),
            (12, 13),
            (13, 17),
            (13, 14),
            (14, 18),
            (14, 15),
            (15, 19),
        ]

        vertex_boundary = [
            True,
            True,
            True,
            True,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            True,
            True,
            True,
            True,
        ]
        num_vertices = len(vertex_boundary)
        dg = pcg.DecodingGraph(num_vertices, edges, vertex_boundary)
        return num_vertices, dg

    def test_seed_0(self, get_decoding_graph):
        syndrome = [
            False,
            False,
            False,
            False,
            False,
            False,
            True,
            False,
            True,
            False,
            True,
            False,
            True,
            True,
            False,
            False,
            False,
            False,
            False,
            False,
        ]
        modified_erasure_check = [
            False,
            False,
            False,
            False,
            True,
            False,
            False,
            False,
            True,
            False,
            False,
            True,
            True,
            True,
            False,
            False,
            False,
            False,
            True,
            True,
            True,
            True,
            False,
            False,
            False,
        ]
        num_vertices, dg = get_decoding_graph
        uf = pcu.UnionFindDecoder(dg)
        uf.decode(syndrome)
        mf = uf.get_modified_erasure()

        for i in range(num_vertices):
            assert mf[i] == modified_erasure_check[i]
