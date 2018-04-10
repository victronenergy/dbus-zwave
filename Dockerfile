# Software versions
FROM debian:latest
ENV SDK_URL https://updates.victronenergy.com/feeds/venus/release/sdk/venus-jethro-x86_64-arm-cortexa8hf-neon-toolchain-qte-v2.12.sh

# Install dependencies
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    chrpath \
    diffstat \
    libdbus-1-dev \
    libevent-dev \
    libudev-dev \
    gawk \
    gcc-multilib \
    git-core \
    libsdl1.2-dev \
    python \
    qt5-default \
    socat \
    texinfo \
    unzip \
    wget

# Install venus SDK
RUN wget -O /tmp/venus-sdk.sh $SDK_URL && \
    chmod u+x /tmp/venus-sdk.sh && \
    /tmp/venus-sdk.sh && \
    rm -f /tmp/venus-sdk.sh && \
    ln -s $(find /opt/venus -mindepth 1 -maxdepth 1) /opt/venus/current

# Set workdir
WORKDIR /workspace

# Copy external dependencies
COPY ext/ ext/

# Build type
ARG BUILD=release

# Build open-zwave and install as system library
#RUN cd ext/open-zwave && make BUILD=$BUILD && make install
RUN bash -c "cd ext/open-zwave && . /opt/venus/current/environment-setup-cortexa8hf-vfp-neon-ve-linux-gnueabi && \
    make BITBAKE_ENV=1 BUILD=$BUILD && \
    env | grep '^PKG_CONFIG_SYSROOT_DIR=' | sed 's/PKG_CONFIG_SYSROOT_DIR/DESTDIR/' | xargs -I {} make {} install"

# Copy rest of sources
COPY configure dbus-zwave.pro rules.mk ./
COPY inc/ inc/
COPY src/ src/

# Build dbus-zwave
#RUN ./configure && make BUILD=$BUILD
#RUN qmake CONFIG+=$BUILD && make
RUN bash -c ". /opt/venus/current/environment-setup-cortexa8hf-vfp-neon-ve-linux-gnueabi && \
    /opt/venus/current/sysroots/x86_64-ve-linux/usr/bin/qmake CONFIG+=$BUILD && \
    make"

# Run app
#CMD LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pkg-config --libs-only-L libopenzwave | sed 's/^-L//') ./dbus-zwave
CMD file ./dbus-zwave >&2 && cat dbus-zwave
