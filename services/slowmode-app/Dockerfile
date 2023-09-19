FROM auterion/app-base:v2

COPY . ./app/

RUN cmake S ./app/ -B /app/build
RUN cmake --build ./app/build -j 8

WORKDIR /app/build
CMD ./slowmode-app-tcp
