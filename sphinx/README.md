# sphinxを使ってドキュメントを作る (復元)

## 最初の手順

今回は、`docs/_sources` にその残骸が残っていたので、これを使ってみる。拡張子は `<hoge>.rst.txt` になっているので、拡張子の `.txt` を削除してそのままコピー

つぎに、sphinxのインストールと起動

```bash
pip install -U sphinx
cd sphinx
sphinx-quickstart
```

## テーマの変更


```bash
pip install sphinx_rtd_theme
```

で、`conf.py` の以下をこう変更

```python
html_theme = 'sphinx_rtd_theme'
```