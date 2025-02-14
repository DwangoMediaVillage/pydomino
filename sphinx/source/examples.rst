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
        (0.0, 0.10999999940395355, "pau"),
        (0.10999999940395355, 0.14000000059604645, "d"),
        (0.14000000059604645, 0.17000000178813934, "o"),
        (0.17000000178813934, 0.20999999344348907, "w"),
        (0.20999999344348907, 0.36000001430511475, "a"),
        (0.36000001430511475, 0.44999998807907104, "N"),
        (0.44999998807907104, 0.49000000953674316, "g"),
        (0.49000000953674316, 0.6200000047683716, "o"),
        (0.6200000047683716, 0.754687488079071, "pau"),
    ]

ここで、第3引数の最小割り当てフレーム数を5に変えてみると、とすると、以下のアラインメント結果が得られます

.. code-block:: python

    alignment_result: list[tuple[float, float, str]] = alignmer.align(wav_source, " ".join(phonemes), 5)
    print(f"{alignment_result}")

.. code-block:: python

    [
        (0.0, 0.029999999329447746, "pau"),
        (0.029999999329447746, 0.10999999940395355, "d"),
        (0.10999999940395355, 0.1599999964237213, "o"),
        (0.1599999964237213, 0.20999999344348907, "w"),
        (0.20999999344348907, 0.36000001430511475, "a"),
        (0.36000001430511475, 0.4399999976158142, "N"),
        (0.4399999976158142, 0.49000000953674316, "g"),
        (0.49000000953674316, 0.6200000047683716, "o"),
        (0.6200000047683716, 0.754687488079071, "pau"),
    ]

音素 "w" に割り当てられた秒数が 0.03秒から0.05秒に伸びます。これが、最小割り当てフレーム数の保証によるものです。このフレーム数の保証は両端の `pau` トークンには適用されていません。このことは、最小割り当てフレーム数を5に変えてみたときの最初の `pau` トークンに割り当てられた秒数が0.03秒であることからもわかります。




コマンドラインツールの場合
**************************

ライブラリのインストールによって、仮想環境にコマンドラインツールもビルドされます。これによってアラインメントの実行も可能です

たとえば

.. code-block:: bash

    $ domino --input_path example/dowaNgo.wav --input_phoneme "pau d o w a N g o pau" --output_path result.lab -N 5

とすると、以下のアラインメント結果が labファイル（result.lab）に出力されます 

.. code-block:: bash

    $ cat result.lab

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
