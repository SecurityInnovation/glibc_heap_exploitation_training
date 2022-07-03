FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

# Update
RUN apt-get update -y && apt-get install socat -y gdb vim tmux python3 python3-pip 

# General things needed for pwntools and pwndbg to run
RUN apt-get install git build-essential libssl-dev libffi-dev libxml2-dev libxslt1-dev zlib1g-dev patchelf python3-dev -y 

RUN pip3 install pwn

# Install pwndbg
RUN git clone https://github.com/pwndbg/pwndbg && cd pwndbg && ./setup.sh && cd ../ 

RUN echo "set auto-load safe-path /" >> /root/.gdbinit

# Challenge files to ADD
RUN git clone https://github.com/mdulin2/house-of-muney 

# Fixes the loader and recompiles the binary for us :) 
RUN cd house-of-muney && ./compile.sh

