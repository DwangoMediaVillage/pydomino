import numpy as np
from pydomino.pydomino_cpp import Aligner_cpp


class Aligner(Aligner_cpp):
    def __init__(self, onnxfile: str):
        super().__init__(onnxfile)

    def align(self, waveform: np.ndarray, phonemes: str, min_aligned_timeframe: int):
        return super().align(waveform, phonemes, min_aligned_timeframe)

    def release(self):
        return super().release()
