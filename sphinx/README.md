# sphinxを使ってドキュメントを作る (復元)

## 最初の手順

今回は、`docs/_sources` にその残骸が残っていたので、これを使ってみる。拡張子は `<hoge>.rst.txt` になっているので、拡張子の `.txt` を削除してそのままコピー
だが、`sphinx-quickstart` の実行時に `index.rst` を作るが、`sphinx-quickstart` にファイルを上書きする機能がないため、index.rstだけは削除しておく必要がある

つぎに、sphinxのインストールと起動

```bash
pip install -U sphinx
cd sphinx
sphinx-quickstart
```

とりまビルド

```bash
cd sphinx
make html
```

## テーマの変更


```bash
pip install sphinx_rtd_theme
```

で、`conf.py` の以下をこう変更

```python
html_theme = 'sphinx_rtd_theme'
```


## ビルド成果物ドキュメントページをdocsディレクトリに移動する

注意点：最終的にgithub pagesで公開することを考えているため、`.nojekyll` の名前の空ファイルが必要

```
cd sphinx
rm -r ../docs/*
cp -a ./build/html/* ../docs
```

なんか隠しファイルは `rm -r` で消せないっぽい？