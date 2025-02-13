import numpy as np
from pydomino.pydomino_cpp import Aligner_cpp


class Aligner(Aligner_cpp):
    def __init__(self, onnxfile: str):
        """コンストラクタ。ここで `onnxfile` で指定したONNXファイルを読み込む

        Args:
            onnxfile (str): 読み込ませたいONNXファイルパス
        """
        super().__init__(onnxfile)

    def __del__(self):
        super().release()

    def align(
        self, waveform_mono_16kHz: np.ndarray, phonemes: str, min_aligned_timeframe: int
    ) -> list[tuple[float, float, str]]:
        """音素遷移予測に基づく日本語音素アラインメントを実行する関数

        Args:
            waveform_mono_16kHz (np.ndarray): 16kHzのモノラル音声信号
            phonemes (str):
            min_aligned_timeframe (int): _description_

        Returns:
            _type_: _description_
        """
        return super().align(waveform_mono_16kHz, phonemes, min_aligned_timeframe)
