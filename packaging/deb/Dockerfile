FROM gcc:12.1-bullseye

RUN mkdir -p /usr/src/spm
COPY . /usr/src/spm
WORKDIR /usr/src/spm
RUN apt update && apt upgrade -y
RUN apt install -y apt-utils
RUN apt install -y make build-essential binutils lintian debhelper dh-make devscripts
RUN chmod +x ./deb-build.sh

CMD ["./packaging/deb/deb-build.sh"]

