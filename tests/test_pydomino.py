""" pydominoのライブラリをpip install してちゃんと動くかどうか
"""


def test_canrun_library():
    from pathlib import Path

    import librosa
    import pydomino

    data = [
        (Path("tests/wavdata/dowaNgo.wav"), ["d", "o", "w", "a", "N", "g", "o"]),
        (Path("tests/wavdata/ishIkI.wav"), ["i", "sh", "i", "k", "i"]),
        (Path("tests/wavdata/tasuuketsU.wav"), ["t", "a", "s", "u", "u", "k", "e", "ts", "u"]),
    ]
    aligner = pydomino.Aligner("onnx_model/phoneme_trantision_model_2.onnx")

    output_dir: Path = Path("tests/results_lib")
    output_dir.mkdir(exist_ok=True)

    for wavfile, phonemes in data:
        y, sr = librosa.load(wavfile, sr=16_000)

        result: list[tuple[float, float, str]] = aligner.align(y, " ".join(phonemes), 3)
        with (output_dir / f"{wavfile.stem}.lab").open("wt") as fp:
            for x in result:
                fp.write(f"{x[0]:.3f}\t{x[1]:.3f}\t{x[2]}\n")


def test_canrun_cli():
    import subprocess
    import os
    import time
    from pathlib import Path

    data = [
        (Path("tests/wavdata/dowaNgo.wav"), ["d", "o", "w", "a", "N", "g", "o"]),
        (Path("tests/wavdata/ishIkI.wav"), ["i", "sh", "i", "k", "i"]),
        (Path("tests/wavdata/tasuuketsU.wav"), ["t", "a", "s", "u", "u", "k", "e", "ts", "u"]),
    ]
    expected_output_dir: Path = Path("tests/expected_results_lib")
    cli_output_dir: Path = Path("tests/results_cli")
    cli_output_dir.mkdir(exist_ok=True)

    for wavfile, phonemes in data:
        cli_labfile: Path = cli_output_dir / (wavfile.stem + ".lab")
        if os.name == "nt":
            status = subprocess.run(
                [
                    "domino.exe",
                    f'--input_path="{str(wavfile)}"',
                    f"--input_phoneme=\"{' '.join(phonemes)}\"",
                    f'--output_path="{str(cli_labfile)}"',
                    f"--min_frame=3",
                ]
            )
            # windowsで動かすと、subprocess.run の箇所の完了を待ってくれなかったため、sleepで擬似的に待機する
            time.sleep(1.0)
        else:
            status = subprocess.run(
                [
                    "domino",
                    f"--input_path={str(wavfile)}",
                    f"--input_phoneme={' '.join(phonemes)}",
                    f"--output_path={str(cli_labfile)}",
                    f"--min_frame=3",
                ]
            )
        assert status.returncode == 0, f"{status.stderr.decode()}"
        assert cli_labfile.read_text() == (expected_output_dir / cli_labfile.name).read_text()
