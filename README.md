# pydomino

`pydomino` は日本語音声に対して音素ラベルをアラインメントするためのツールです。GPUは不要です。
ライブラリとして Python から使うこともコマンドラインツールとしてコンソールから使うこともできます。
ドキュメントは [こちら](https://dwangomediavillage.github.io/pydomino/) からご覧いただけます。

## Installation

### Requisites

- CMake
- Python >= 3.10 (miniconda etc.)
- Visual Studio >= 2019 (for Windows)

### Build & Install

#### Linux / Mac

```sh
git clone --recursive {this-repository-url}
cd pydomino
pip install ./
```

また、下記のように直接 pip インストールもできます（コマンドラインツールはインストールされません）：

```sh
pip install git+{this-repository-url}
```


#### Windows

`Anaconda Prompt (miniconda3)` 環境において MSVC の `vcvars64.bat` を利用してインストールします：

* `"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"` or
* `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`.

```sh
# on `Anaconda Prompt (miniconda3)`
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
git clone --recursive {this-repository-url}
cd pydomino
pip install ./
```

## Run samples

### Python Library

```py
alignmer: pydomino.Aligner = pydomino.Aligner(path-to-model-file.onnx)

y: np.ndarray = librosa.load(path-to-wav-file, sr=16_000, mono=True, dtype=np.float32)[0]
p: list[str] = path-to-phoneme-file.read_text().split(" ")
z: list[tuple[float, float, str]] = alignmer.align(y, " ".join(p), N=3) # [(start_time_sec, end_time_sec, phoneme_str)]
```

* `path-to-model-file.onnx` は事前学習済みの onnx モデルファイルです。
  * `onnx_model/model.onnx`にあります。
* `path-to-wav-file` はサンプリング周波数 16kHz のモノラル wav ファイルです。
* `path-to-phoneme-file` は音素を空白区切りしたテキストが格納されたファイルのパスです。
  * NOTE: 開始音素と終了音素は `pau` である必要があります。

`phonemes` に使える音素一覧は下記の通りです：

|       |      |     |      |     |      |     |      |      |      |
| ----- | ---- | --- | ---- | --- | ---- | --- | ---- | ---- | ---- |
| `pau` | `ry` | `r` | `my` | `m` | `ny` | `n` | `j`  | `z`  | `by` |
| `b`   | `dy` | `d` | `gy` | `g` | `ky` | `k` | `ch` | `ts` | `sh` |
| `s`   | `hy` | `h` | `v`  | `f` | `py` | `p` | `t`  | `y`  | `w`  |
| `N`   | `a`  | `i` | `u`  | `e` | `o`  | `I` | `U`  | `cl` |      |

### Console Application

上記のインストール手順における `pip install` により Cli ツールも自動でビルドされます。

ビルドされたツールは下記のようにして使えます：

```sh
./build/{temporary-directory}/pydomino/domino \
    --input_path={path-to-wav-file} \
    --input_phoneme={path-to-phoneme-file} \
    --output_path={path-to-output-lab-file} \
    --model={path-to-model-file.onnx} \
    -N=3
```

### label file format (.lab)

アラインメント結果のラベルファイル (.lab) は、tsv ファイル構造になっています。

各行に音素の開始時刻と終了時刻 (いずれも単位は秒) と、そのときの音素が TAB 区切りで並んでいます：

```txt
0.00    1.87    pau
1.87    1.90    t
1.90    3.10    pau
```