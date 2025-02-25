.. pydomino documentation master file, created by
   sphinx-quickstart on Thu Feb 13 19:20:32 2025.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

日本語音素アラインメントツール pydomino
=======================================

pydominoとは
------------

pydomino は日本語音声に対して音素ラベルをアラインメントするためのツールです。

使い方
------

pydomino はPythonライブラリとコマンドラインツールの2通りで利用できます。

インストール方法
----------------

Linux / Mac
***********

.. code-block::bash

   git clone --recursive {this-repository-url}
   pip install ./

コマンドラインツールがいらない場合はこちらでも直接ライブラリだけインストールできます

.. code-block:: bash

   pip install git+{this-repository-url}



Windows
*******

Anaconda Prompt (miniconda3) 環境において MSVC の vcvars64.bat を利用してインストールします

例えば、以下のコマンドでインストールできます

.. code-block:: bash

   # on `Anaconda Prompt (miniconda3)`
   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
   git clone --recursive {this-repository-url}
   pip install ./


例示したパスに vcvars64.bat がないなら、例えば以下のような場所にあるかもしれません。

.. code-block:: bash

   "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" or
   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat".


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   pydomino
   examples
   how_to_make_onnxfile



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


