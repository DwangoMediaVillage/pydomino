ONNXファイルを自分で作るには
============================

このページでは、自前で学習したTransformer音素遷移予測ネットワークをONNXファイルとして書き出すときのインタフェースを記述します

ONNXバージョン情報
------------------
ONNXは1.16.3 を使っています


入力
----

入力は2次元行列のモノラル音声信号です。変数名は `input_waveform`、行列のshapeは `(1 x sample_length)` で、`sample_length` は可変長にしています。

出力
----

出力は音素遷移トークンの対数発火確率と、blankトークンの対数発火確率の2つです。

音素遷移トークンの対数発火確率
******************************

3次元行列です。変数名は `transition_logprobs`、行列のshapeは `(1 x seq_length x num_token_vocab)` で、`seq_length` は可変長にしています。

`seq_length` は時間フレームの総数で、`sample_length` から決まります。
`num_token_vocab` は予測対象の音素遷移トークンの種類数です。種類とその順番は `src/phoneme_transitions.txt` で指定します。


blankトークンの対数発火確率
***************************

2次元行列です。変数名は `blank_logprobs`、行列のshapeは `(1 x seq_length)` で、`seq_length` は可変長にしています。

`seq_length` は時間フレームの総数で、音素遷移トークンの対数発火確率と同じ値です。