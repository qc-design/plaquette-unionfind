import warnings

warnings.filterwarnings("ignore")
import pytest
import plaquette_unionfind as pcu
import plaquette_graph as pcg

import sys
from typing import Type, cast

import numpy as np
import pytest as pt

import plaquette
from plaquette.circuit.generator import generate_qec_circuit
from plaquette.codes import LatticeCode
from plaquette import Device
from plaquette.decoders import (
    FusionBlossomDecoder,
    PyMatchingDecoder,
    UnionFindDecoder,
    decoderbase,
)
from plaquette.errors import (
    ErrorDataDict,
    ErrorValueDict,
    GateErrorsDict,
    QubitErrorsDict,
    SinglePauliChannelErrorValueDict,
)
from plaquette.frontend import ExperimentConfig
from plaquette.device import MeasurementSample
from plaquette.syngraph import SyndromeGraph
from plaquette_graph import *
import plaquette_unionfind
from plaquette.decoders.decoderbase import check_success



plaquette.rng = np.random.default_rng(seed=1234567890)
sys.setrecursionlimit(10000)

size = 8
error_rate = 0.09973
expected_success_rate = 0.8489
expected_success_rate_error = 0.0033  # 3-sigma
logical_op = "Z"
reps = 96000

code = LatticeCode.make_planar(n_rounds=1, size=size)
qed = {"pauli": {q.equbit_idx: {"x": error_rate} for q in code.lattice.dataqubits}}
circuit = generate_qec_circuit(code, qed, {}, logical_op)
dev = Device("stim", batch_size=reps)
successes = 0

for i in range(reps):
    dev.run(circuit)
    raw_results, erasure = dev.get_sample()
    sample = MeasurementSample.from_code_and_raw_results(code, raw_results, erasure)

    decoder = plaquette_unionfind.UnionFindDecoderInterface.from_code(
        code, qed, weighted=False
    )
    correction = decoder.decode(sample.erased_qubits, sample.syndrome)

    if check_success(code, correction, sample.logical_op_toggle, logical_op):
        successes += 1

success_rate = successes / reps
assert (
    expected_success_rate - expected_success_rate_error / 2
    <= success_rate
    <= expected_success_rate + expected_success_rate_error / 2
)
