
ARG OS=ubuntu:24.04
ARG COMPILER=gcc-14
ARG PRESET=vrdev_gcc

FROM ${OS} AS setup-cpp

ARG COMPILER

RUN \
  apt-get update \
  && apt-get install -y --no-install-recommends nodejs npm git \
  && apt-get install -y --no-install-recommends libxrandr-dev libxi-dev libxcursor-dev libglfw3-dev libudev-dev libfreetype-dev libogg-dev libvorbis-dev libflac-dev \
  && npm install -g setup-cpp@v0.38.3 \
  && setup-cpp \
    --compiler ${COMPILER} \
    --cmake true \
    --ninja true \
    --ccache true \
    --make true \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /tmp/*

RUN \
  mkdir -p /root/work

WORKDIR /root/work

RUN \
  git clone https://github.com/menuet/VRSFML.git

WORKDIR /root/work/VRSFML

RUN \
  git checkout try-fix-build

ARG PRESET
ENV PRESET=${PRESET}

RUN \
  bash -c 'source ~/.cpprc && cmake --preset ${PRESET} && cmake --build build_${PRESET}'
