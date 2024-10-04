FROM fedora:40

RUN yum -y update &&  \
    yum -y install make g++ gcc gdb qemu binutils binutils-x86_64-linux-gnu cross-gcc-common \
                    gcc-c++-x86_64-linux-gnu nano dev86 bridge-utils libvirt virt-install qemu-kvm && \
    yum clean all

WORKDIR /app

ENTRYPOINT ["/sbin/init"]
