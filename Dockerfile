FROM ubuntu:23.04
LABEL maintainer="camil.ziane@epita.fr"

RUN apt-get update \
    && apt-get install -y \
     cmake \
     clang-14 \
     clang-format \
     g++-11 \
     git \
     make \
     python3 \
     python3-dev \
     python3-pip \
     python3-yaml \
     valgrind \
     nlohmann-json3-dev

RUN apt-get install -y wget

RUN cd /home && wget https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.gz \
  && tar xfz boost_1_79_0.tar.gz \
  && rm boost_1_79_0.tar.gz \
  && cd boost_1_79_0 \
  && ./bootstrap.sh --prefix=/usr/local --with-libraries=program_options \
  && ./b2 install \
  && cd /home \
  && rm -rf boost_1_79_0

RUN apt-get install -y python3-venv

CMD bash