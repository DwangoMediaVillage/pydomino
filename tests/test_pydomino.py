""" pydominoのライブラリをpip install してちゃんと動くかどうか """


def test_canrun():
    from pathlib import Path

    import librosa
    import pydomino

    data = [
        (Path("tests/wavdata/dowaNgo.wav"), ["d", "o", "w", "a", "N", "g", "o"]),
        (Path("tests/wavdata/ishIkI.wav"), ["i", "sh", "i", "k", "i"]),
        (Path("tests/wavdata/tasuuketsU.wav"), ["t", "a", "s", "u", "u", "k", "e", "ts", "u"]),
    ]
    aligner = pydomino.Aligner("onnx_model/phoneme_trantision_model_2.onnx")

    for wavfile, phonemes in data:
        y, sr = librosa.load(wavfile, sr=16_000)

        result: list[tuple[float, float, str]] = aligner.align(y, " ".join(phonemes), 3)
        for x in result:
            print(f"{x}")
