# sphinxを使ってドキュメントを作る

## ライブラリのインストールとビルド方法

事前にライブラリインストールが必要です。

```bash
pip install -U sphinx sphinx_rtd_theme
```

ビルド方法は、`make html` です。

```bash
make html
```

## ビルド成果物ドキュメントページをdocsディレクトリに移動する

注意点：github pagesで公開することを考えているため、`.nojekyll` の名前の空ファイルが必要です

