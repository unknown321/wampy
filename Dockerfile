FROM nw-crosstool
ARG CMAKE_RELEASE_VERSION=3.25.3
ENV CMAKE_RELEASE_VERSION=$CMAKE_RELEASE_VERSION
RUN apt-get build-dep qtbase5-dev -y && \
    apt-get install e2tools etc1tool -y && \
    apt-get clean && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    cd /opt && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_RELEASE_VERSION}/cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64.tar.gz && \
    tar -xf cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64.tar.gz && \
    rm cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64.tar.gz && \
    ln -s /opt/cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64/bin/cmake /usr/bin/cmake
