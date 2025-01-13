FROM nw-crosstool
ARG CMAKE_RELEASE_VERSION=3.25.3
ENV CMAKE_RELEASE_VERSION=$CMAKE_RELEASE_VERSION
RUN apt-get build-dep qtbase5-dev -y && \
    apt-get install e2tools -y && \
    apt-get clean && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    cd /opt && \
    wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_RELEASE_VERSION}/cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64.tar.gz && \
    tar -xf cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64.tar.gz && \
    rm cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64.tar.gz && \
    ln -s /opt/cmake-${CMAKE_RELEASE_VERSION}-linux-x86_64/bin/cmake /usr/bin/cmake

RUN ln -s /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-ld /usr/bin/arm-linux-gnueabihf-ld && \
		ln -s /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-ld.bfd /usr/bin/arm-linux-gnueabihf-ld.bfd && \
		ln -s /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-gcc /usr/bin/arm-linux-gnueabihf-gcc && \
		ln -s /x-tools/armv5-unknown-linux-gnueabihf/bin/armv5-unknown-linux-gnueabihf-strip /usr/bin/arm-linux-gnueabihf-strip