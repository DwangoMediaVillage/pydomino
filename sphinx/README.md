# sphinxを使ってドキュメントを作る (復元)

## 最初の手順

今回は、`docs/_sources` にその残骸が残っていたので、これを使ってみる。拡張子は `<hoge>.rst.txt` になっているので、拡張子の `.txt` を削除してそのままコピー

つぎに、sphinxのインストールと起動

```bash
pip install -U sphinx
sphinx-quickstart
```