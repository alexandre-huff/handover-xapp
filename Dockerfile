FROM nexus3.o-ran-sc.org:10002/o-ran-sc/bldr-ubuntu22-c-go:1.0.0 as buildenv

RUN mkdir -p /playpen/handover-xapp

WORKDIR /playpen

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    gdb \
    libcpprest-dev \
    libcpprest \
    meson

# install pistache
RUN git clone --depth 1 https://github.com/pistacheio/pistache.git \
    && cd pistache \
    && meson setup build --prefix=/usr/local --libdir=lib -Ddebug=true \
    && meson install -C build \
    && ldconfig

# install nlohmann json
RUN git clone --depth 1 https://github.com/nlohmann/json.git nlohmann \
    && mkdir -p nlohmann/build \
    && cd nlohmann/build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DJSON_BuildTests=OFF \
    && make \
    && make install \
    && ldconfig

# install json-schema-validator
RUN git clone https://github.com/pboettch/json-schema-validator.git \
    && cd json-schema-validator \
    && git checkout cae6fad80001510077a7f40e68477a31ec443add \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j`nproc` install

# install rapidjson
RUN git clone https://github.com/Tencent/rapidjson.git \
    && cp -r rapidjson/include/rapidjson /usr/local/include/

# download RMR
ARG RMR_VER=4.9.4
RUN wget --content-disposition "https://packagecloud.io/o-ran-sc/release/packages/debian/stretch/rmr-dev_${RMR_VER}_amd64.deb/download.deb?distro_version_id=149" \
    && wget --content-disposition "https://packagecloud.io/o-ran-sc/release/packages/debian/stretch/rmr_${RMR_VER}_amd64.deb/download.deb?distro_version_id=149"

# download MDC Logger
ARG MDC_VER=0.1.4-1
RUN wget --content-disposition "https://packagecloud.io/o-ran-sc/release/packages/debian/stretch/mdclog_${MDC_VER}_amd64.deb/download.deb?distro_version_id=149" && \
    wget --content-disposition "https://packagecloud.io/o-ran-sc/release/packages/debian/stretch/mdclog-dev_${MDC_VER}_amd64.deb/download.deb?distro_version_id=149"

COPY *.deb /playpen
RUN dpkg -i /playpen/*.deb

# clone xapp-frame-cpp and apply patch
COPY xapp-frame-cpp.patch /playpen/handover-xapp
RUN git clone "https://gerrit.o-ran-sc.org/r/ric-plt/xapp-frame-cpp" \
    && cd xapp-frame-cpp \
    && git checkout 2b74c8b7baebe36501d77f6848f25dc74ab64fc8 \
    && git apply /playpen/handover-xapp/xapp-frame-cpp.patch

# install xapp-frame-cpp
RUN cd xapp-frame-cpp \
    && rm -rf .build \
    && mkdir .build \
    && cd .build \
    && cmake .. -DDEV_PKG=1 \
    && make -j`nproc` install \
    && cmake .. -DDEV_PKG=0 \
    && make -j`nproc` install

COPY . /playpen/handover-xapp/

WORKDIR /playpen/handover-xapp

RUN mkdir -p build && cd build \
    && cmake .. \
    && make -j`nproc` \
    && make install


# build the final and smaller image
FROM ubuntu:22.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libcpprest \
    && apt-get clean \
    && mkdir -p /usr/local/etc/handover-xapp

COPY --from=buildenv /usr/local/bin /usr/local/bin
COPY --from=buildenv /usr/local/lib /usr/local/lib
COPY --from=buildenv /playpen/handover-xapp/init/config-map.yaml /usr/local/etc/handover-xapp
COPY --from=buildenv /playpen/handover-xapp/init/routes.txt /usr/local/etc/handover-xapp

# copy preserving symlinks
RUN --mount=type=bind,from=buildenv,source=/usr/lib/x86_64-linux-gnu,target=/context cp -av /context/libmdclog.so* /usr/lib/x86_64-linux-gnu/

ENV CONFIG_MAP_NAME=/usr/local/etc/handover-xapp/config-map.yaml
ENV RMR_SEED_RT=/usr/local/etc/handover-xapp/routes.txt

RUN ldconfig

# CMD while true; do sleep 3600; done
CMD handover-xapp
