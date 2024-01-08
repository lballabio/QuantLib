ARG tag=latest
FROM ubuntu:${tag}
MAINTAINER Luigi Ballabio <luigi.ballabio@gmail.com>
LABEL Description="Provide Docker images for QuantLib's CI builds on Linux"

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential wget libbz2-dev autoconf automake libtool ccache cmake clang git \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

ARG boost_version
ARG boost_dir
ENV boost_version ${boost_version}

RUN wget https://downloads.sourceforge.net/project/boost/boost/${boost_version}/${boost_dir}.tar.gz \
    && tar xfz ${boost_dir}.tar.gz \
    && rm ${boost_dir}.tar.gz \
    && cd ${boost_dir} \
    && ./bootstrap.sh \
    && ./b2 --without-python --prefix=/usr -j 4 link=shared runtime-link=shared install \
    && cd .. && rm -rf ${boost_dir} && ldconfig

CMD bash

