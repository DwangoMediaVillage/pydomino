pydominoの使用例
================

pythonライブラリの場合
**********************


.. code-block:: python

    import numpy as np
    import librosa
    import pydomino

    alignmer: pydomino.Aligner = pydomino.Aligner("onnx_model/phoneme_transition_model.onnx")
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

    domino \
        --input_path example/dowaNgo.wav \  
        --input_phoneme "pau d o w a N g o pau" \   
        --output_path result.lab \
        --onnx_path=onnx_model/phoneme_transition_model.onnx \
        --min_frame=3

とすると、以下のアラインメント結果が labファイル（result.lab）に出力されます 

.. code-block:: bash

    $ cat result.lab

.. code-block:: guess

    0.000	0.000	pau
    0.000	0.080	d
    0.080	0.150	o
    0.150	0.190	w
    0.190	0.350	a
    0.350	0.460	N
    0.460	0.490	g
    0.490	0.640	o
    0.640	0.724	pau


TextGridファイルへの出力 (version 1.2.1以降)
-----------------------------------------------


コマンドラインツールを使った場合に限ってですが、TextGridファイルの出力に対応しました。

オプション `--output_format=TextGrid` を付け加えると、出力が lab ファイルではなく TextGrid ファイルとなります

.. code-block:: bash

    domino \
        --input_path example/dowaNgo.wav \  
        --input_phoneme "pau d o w a N g o pau" \   
        --output_path result.lab \
        --onnx_path=onnx_model/phoneme_transition_model.onnx \
        --output_format=TextGrid \
        --min_frame=3

とすると、以下のアラインメント結果が labファイル（result.TextGrid）に出力されます 

.. code-block:: bash

    $ cat result.TextGrid

.. code-block:: guess

    File type = "ooTextFile"
    Object class = "TextGrid"

    xmin = 0
    xmax = 0.724
    tiers? <exists>
    size = 2
    item []:
        item [1]:
            class = "IntervalTier"
            name = "phonemes"
            xmin = 0
            xmax = 0.724
            intervals: size = 1
            intervals [1]:
                xmin = 0
                xmax = 0.724
                text = "d o w a N g o pau"
        item [2]:
            class = "IntervalTier"
            name = "alignment result"
            xmin = 0
            xmax = 0.724
            intervals: size = 8
            intervals[1]:
                xmin = 0.000
                xmax = 0.080
                text = "d"
            intervals[2]:
                xmin = 0.080
                xmax = 0.150
                text = "o"
            intervals[3]:
                xmin = 0.150
                xmax = 0.190
                text = "w"
            intervals[4]:
                xmin = 0.190
                xmax = 0.350
                text = "a"
            intervals[5]:
                xmin = 0.350
                xmax = 0.460
                text = "N"
            intervals[6]:
                xmin = 0.460
                xmax = 0.490
                text = "g"
            intervals[7]:
                xmin = 0.490
                xmax = 0.640
                text = "o"
            intervals[8]:
                xmin = 0.640
                xmax = 0.724
                text = "pau"

