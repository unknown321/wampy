FROM silkeh/clang:7@sha256:312aaa0af60de25c058298423b1dc980c2051ca619f0c7dcfcbd1550dd23e2c1
RUN echo "deb http://snapshot.debian.org/archive/debian/20240612T000000Z buster main" > /etc/apt/sources.list && \
    echo "#deb http://deb.debian.org/debian buster main" >> /etc/apt/sources.list && \
    echo "#deb http://snapshot.debian.org/archive/debian-security/20240612T000000Z buster/updates main" >> /etc/apt/sources.list && \
    echo "#deb http://deb.debian.org/debian-security buster/updates main" >> /etc/apt/sources.list && \
    echo "#deb http://snapshot.debian.org/archive/debian/20240612T000000Z buster-updates main" >> /etc/apt/sources.list && \
    echo "#deb http://deb.debian.org/debian buster-updates main" >> /etc/apt/sources.list && \
    apt-get update && apt-get install -y \
    gcc \
    make \
    gcc-arm-linux-gnueabi \
    libc6-dev-armhf-cross \
    libstdc++6-armhf-cross \
    binutils-arm-linux-gnueabi \
    libc++-dev libc++abi-dev \
    crossbuild-essential-armhf && \
    apt-get clean
RUN useradd -U -u 1000 user
USER 1000:1000
