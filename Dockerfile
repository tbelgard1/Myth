# Stage 1: Build
FROM ubuntu:22.04 as builder

RUN apt update && apt install -y build-essential gcc g++ make git

WORKDIR /src
COPY spaghetti/ ./spaghetti

WORKDIR /src/spaghetti
RUN cd common && make && cd ../users_new && make clean && make

# Stage 2: Runtime
FROM ubuntu:22.04

COPY --from=builder /src/spaghetti/users_new/userd_new /bin/userd_new
COPY startup.sh /startup.sh

RUN chmod +x /startup.sh
CMD ["/startup.sh"]
