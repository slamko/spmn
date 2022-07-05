FROM gcc:12.1-bullseye

RUN mkdir -p /usr/src/spmn
COPY . /usr/src/spmn
WORKDIR /usr/src/spmn
RUN apt update && apt upgrade -y
RUN apt install -y apt-utils
RUN apt install -y make build-essential binutils lintian debhelper dh-make devscripts libbsd0 libbsd-dev
