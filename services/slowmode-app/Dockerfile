FROM auterion/app-base:v0

COPY . ./app/

RUN cmake S ./app/ -B /app/build
RUN cmake --build ./app/build -j 8

# CMD ./app/build/b
WORKDIR /app/build
CMD ./slowmode-app
# CMD ["bash"]