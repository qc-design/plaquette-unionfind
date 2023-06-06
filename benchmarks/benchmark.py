import sys
import numpy as np
import plaquette
import benchmark_helpers

import plaquette
from plaquette import syngraph
from plaquette import Device
from plaquette.decoders import decoderbase
from plaquette.codes import LatticeCode
from plaquette.errors import QubitErrorsDict
from plaquette.device import MeasurementSample
from plaquette.codes import LatticeCode
from plaquette.circuit.generator import generate_qec_circuit

if (len(sys.argv) != 7):
    print("Usage: code_type size measurement_type p decoder_type seed")
    print("code_type: planar, rotated_planar")
    print("decoder_type: pymatching, fusion-blossom, plaquette-unionfind")
    print("measurement_type: perfect, imperfect")
    exit(1)

code_type = sys.argv[1]
size = int(sys.argv[2])
measurement_type = str(sys.argv[3])
p = float(sys.argv[4])
decoder_type = sys.argv[5]
seed = int(sys.argv[6])

plaquette.rng = np.random.default_rng(seed=seed)
logical_op = "Z"

code, qed, ged = benchmark_helpers.benchmark_code_qed_ged(
    code_type, size, measurement_type, p
)
circuit = generate_qec_circuit(code, qed, ged, logical_op)
dev = Device("stim", batch_size=1)

dev.run(circuit)
raw_results, erasure = dev.get_sample()
sample = MeasurementSample.from_code_and_raw_results(code, raw_results, erasure)

if decoder_type == "pymatching":
    decoder = benchmark_helpers.PyMatchingDecoderBenchmark.from_code(
        code, qed, weighted=True
    )
elif decoder_type == "fusion-blossom":
    decoder = benchmark_helpers.FusionBlossomDecoderBenchmark.from_code(
        code, qed, weighted=True
    )
elif decoder_type == "plaquette-unionfind":
    decoder = benchmark_helpers.UnionFindDecoderBenchmark.from_code(
        code, qed, weighted=False
    )
else:
    raise ValueError("Invalid decoder type")

correction = decoder.decode(sample.erased_qubits, sample.syndrome)
