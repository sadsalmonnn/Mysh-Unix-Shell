FROM ubuntu:latest

RUN apt update && \
    apt install -y build-essential

WORKDIR /app

COPY ./src ./src

WORKDIR /app/src

RUN rm -f *.o mysh

RUN make

CMD ["./mysh"]