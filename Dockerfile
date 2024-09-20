FROM fedora:40

ENV APP=hello

RUN yum -y update &&  \
    yum -y install make g++ gcc gdb qemu binutils binutils-x86_64-linux-gnu cross-gcc-common \
                    gcc-c++-x86_64-linux-gnu nano dev86 && \
    yum clean all

WORKDIR /app

CMD ["make", "APPLICATION=$APP", "run"]
