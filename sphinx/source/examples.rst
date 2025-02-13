pydominoの使用例
================

pythonライブラリの場合
**********************


.. code-block:: python

    import numpy as np
    import librosa
    import pydomino

    alignmer: pydomino.Aligner = pydomino.Aligner("onnx_model/model.onnx")
    wav_source: np.ndarray = librosa.load("example/dowaNgo.wav", sr=16_000, mono=True, dtype=np.float32)[0]
    phonemes: list[str] = ["pau"] + ["d", "o", "w", "a", "N", "g", "o"] + ["pau"]
    alignment_result: list[tuple[float, float, str]] = alignmer.align(wav_source, " ".join(phonemes), 3)
    print(f"{alignment_result}")

とすると、以下のアラインメント結果が得られます

.. code-block:: python

    [
        (0.0, 0.07999999821186066, 'pau'),
        (0.07999999821186066, 0.12999999523162842, 'd'),
        (0.12999999523162842, 0.20000000298023224, 'o'),
        (0.20000000298023224, 0.25999999046325684, 'w'),
        (0.25999999046325684, 0.3400000035762787, 'a'),
        (0.3400000035762787, 0.44999998807907104, 'N'),
        (0.44999998807907104, 0.47999998927116394, 'g'),
        (0.47999998927116394, 0.6800000071525574, 'o'),
        (0.6800000071525574, 0.7300000190734863, 'pau')
    ]

ここで、第3引数の最小割り当てフレーム数を5に変えてみると、とすると、以下のアラインメント結果が得られます

.. code-block:: python

    alignment_result: list[tuple[float, float, str]] = alignmer.align(wav_source, " ".join(phonemes), 3)
    print(f"{alignment_result}")

.. code-block:: python

    [
        (0.0, 0.07999999821186066, 'pau'),
        (0.07999999821186066, 0.12999999523162842, 'd'),
        (0.12999999523162842, 0.20000000298023224, 'o'),
        (0.20000000298023224, 0.25999999046325684, 'w'),
        (0.25999999046325684, 0.3400000035762787, 'a'),
        (0.3400000035762787, 0.4399999976158142, 'N'),
        (0.4399999976158142, 0.49000000953674316, 'g'),
        (0.49000000953674316, 0.6800000071525574, 'o'),
        (0.6800000071525574, 0.7300000190734863, 'pau')
    ]

音素 "g" に割り当てられた秒数が 0.03秒から0.05秒に伸びます。これが、最小割り当てフレーム数の保証によるものです。




コマンドラインツールの場合
**************************

git clone と pip install ./ を使った場合、./build ディレクトリ以下にコマンドラインツールがビルドされます。

これによってアラインメントの実行も可能です

たとえば

.. code-block:: bash

    $ build/{temporary-directory}/pydomino/domino --input_path example/dowaNgo.wav --input_phoneme "pau d o w a N g o pau" --output_path result.lab -N 5

とすると、以下のアラインメント結果が labファイル（result.lab）に出力されます 

.. code-block:: bash

    $ cat result.wav

.. code-block:: guess

    0.00	0.08	pau
    0.08	0.13	d
    0.13	0.20	o
    0.20	0.26	w
    0.26	0.34	a
    0.34	0.44	N
    0.44	0.49	g
    0.49	0.68	o
    0.68	0.73	pau
