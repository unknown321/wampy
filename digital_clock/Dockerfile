FROM rust:1.83.0-bullseye
RUN apt-get update && apt-get install unzip pkg-config libssl-dev python3-pip gcc-arm-none-eabi -y && \
    apt-get clean && \
    pip3 install pyfatfs fonttools pillow
