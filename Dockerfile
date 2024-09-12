FROM fedora:40

RUN yum -y update && yum clean all
RUN yum -y install git qemu binutils binutils-x86_64-linux-gnu nano micro nvim vim dev86 \
    && yum clean all

CMD git clone https://gitlab.lisha.ufsc.br/epos/ine5424.git && \
    cd ine5424 && \
    git checkout 2024_2 && \
    /bin/sh
