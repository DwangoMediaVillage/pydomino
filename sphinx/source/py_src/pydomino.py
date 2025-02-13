import numpy as np


class Aligner:

    def __init__(self, onnxfile: str):
        """コンストラクタ。ここで `onnxfile` で指定したONNXファイルを読み込む"""
        pass

    def align(self, waveform_mono_16kHz: np.ndarray, phonemes: str, min_aligned_timeframe: int):
        """_summary_

        Args:
            waveform_mono_16kHz (np.ndarray): _description_
            phonemes (str): _description_
            min_aligned_timeframe (int): _description_

        Returns:
            _type_: _description_
        """
        return super().align(waveform_mono_16kHz, phonemes, min_aligned_timeframe)

    def release(self):
        return super().release()
