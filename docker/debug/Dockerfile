FROM multi2sim/multi2sim:build-env
MAINTAINER NUCAR

# Build Multi2Sim with debug and install gdb
RUN cd /tmp && git clone https://github.com/Multi2Sim/multi2sim.git && cd multi2sim && libtoolize && aclocal && autoconf && automake --add-missing && ./configure --enable-debug && make && make install
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install gdb
