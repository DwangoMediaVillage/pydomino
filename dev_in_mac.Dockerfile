# ベースとなるOSはubuntu 20.04を選択
FROM ubuntu:20.04

# パッケージの一覧更新
RUN apt-get update

# タイムゾーンの設定
RUN apt install -y tzdata
ENV TZ=Asia/Tokyo

# 開発環境のシステムインストール
RUN apt install -y wget \
  g++ \
  cmake \
  git \
  clang-format \
  hdf5-tools \
  hdf5-helpers \
  libhdf5-dev \
  libhdf5-doc \
  libhdf5-serial-dev