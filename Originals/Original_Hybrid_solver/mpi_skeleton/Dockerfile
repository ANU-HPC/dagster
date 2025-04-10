# Build this image:  docker build -t milthorpe/async-neighbours .
#
FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install --no-install-recommends -y wget \
 build-essential \
 make \
 ca-certificates \
 vim
RUN wget https://download.open-mpi.org/release/open-mpi/v4.0/openmpi-4.0.0.tar.bz2
RUN tar -xvf openmpi-4.0.0.tar.bz2
RUN cd openmpi-4.0.0 && ./configure --prefix=/usr/local && make all install
RUN ldconfig

# Don't run everything as root!
ENV USER appuser
RUN useradd -s /bin/bash ${USER}
ENV DEBIAN_FRONTEND=noninteractive \
    HOME=/home/${USER} 

# Install SSHD
RUN apt-get install --no-install-recommends -y openssh-server
RUN mkdir /var/run/sshd
RUN echo 'root:${USER}' | chpasswd
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# SSH login fix. Otherwise user is kicked off after login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

# Setup SSH
ENV SSHDIR ${HOME}/.ssh/
RUN mkdir -p ${SSHDIR}
RUN ssh-keygen -f ${SSHDIR}/id_rsa -t rsa -N ''
RUN cp ${SSHDIR}/id_rsa.pub ${SSHDIR}/authorized_keys
RUN chmod -R 600 ${SSHDIR}* && \
    chown -R ${USER}:${USER} ${SSHDIR}

# ------------------------------------------------------------
# Configure OpenMPI
# ------------------------------------------------------------

USER root

RUN rm -fr ${HOME}/.openmpi && mkdir -p ${HOME}/.openmpi
ADD default-mca-params.conf ${HOME}/.openmpi/mca-params.conf
RUN chown -R ${USER}:${USER} ${HOME}/.openmpi

## Suppress error message 'Could not load host key: ...'
RUN yes y | /usr/bin/ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key -C '' -N ''
RUN yes y | /usr/bin/ssh-keygen -t rsa -f /etc/ssh/ssh_host_dsa_key -C '' -N ''
EXPOSE 22

USER ${USER}
WORKDIR /home/${USER}
