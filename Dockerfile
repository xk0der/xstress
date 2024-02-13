# Use an existing image as a base
FROM ubuntu:latest

# Copy folders from host to image
COPY src          /xstress/src
COPY include      /xstress/include
COPY bin          /xstress/bin 
COPY makefile     /xstress/
COPY build        /xstress
COPY xstress.conf /xstress

# Set the working directory
WORKDIR /xstress

RUN apt-get update && apt-get install -y \
    make \
    gcc \
    g++ \
    neovim


# Run make command
RUN yes "y" | ./build

# Keep running - this a container to test xstress
ENTRYPOINT ["tail", "-f", "/dev/null"]
