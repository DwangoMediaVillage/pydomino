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
            phonemes (str): 半角スペース区切りの音素列
            min_aligned_timeframe (int): 両端にある `pau` 音素以外のすべての音素に割り当てられる最低時間フレーム。1フレーム10ミリ秒なので、N=3ですべての音素が30ミリ秒以上割り当てられる

        Returns:
            list[tuple[float, float, str]]: アラインメント結果。`(開始秒数, 終了秒数, 音素)` のタプル列
        """
        return super().align(waveform_mono_16kHz, phonemes, min_aligned_timeframe)

    def release(self):
        """内部で読み込んだ ONNX ファイルのメモリを開放する関数。デストラクタでこの関数を呼び出す。"""
        super().release()
