FROM auterion/app-base:v2 as build-stage

ARG RESTINIO_VERSION=0.6.18

RUN apt-get update && apt-get install -y \
    libasio-dev \
    libfmt-dev \
    libboost-all-dev \
    libhttp-parser-dev \
    libspdlog-dev

# Install restinio
RUN curl --proto "=https" -L https://github.com/Stiffstream/restinio/releases/download/v.${RESTINIO_VERSION}/restinio-${RESTINIO_VERSION}-full.tar.bz2 -o restinio.tar.bz2 && \
    tar xf restinio.tar.bz2 && \
    cd restinio-${RESTINIO_VERSION}/dev && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DRESTINIO_TEST=no -DRESTINIO_SAMPLE=no -DRESTINIO_INSTALL_SAMPLES=no -DRESTINIO_BENCH=no  .. && \
    make -j8 install && \
    cd ../../.. && \
    rm -rf restinio.zip restinio-${RESTINIO_VERSION}

COPY . ./app/

RUN cmake -S ./app/ -B /app/build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build ./app/build -j 8

FROM auterion/app-base:v2 as run-stage

COPY --from=build-stage /app/build /app/build

WORKDIR /app/build
CMD ./slowmode-app-tcp
