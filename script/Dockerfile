FROM ubuntu:16.04

LABEL maintainer Jalil Modares "info@jmod.io"

RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
            build-essential libgl1-mesa-dev libsdl2-dev \
            python-future libfontconfig1 dbus-x11 geoclue \
            sudo curl git ca-certificates netcat xvfb \
    && rm -rf /var/lib/apt/lists/*

RUN useradd --groups sudo,audio,video,dialout --create-home --shell /bin/bash ub-anc \
    && echo "ub-anc:ub-anc" | chpasswd

USER ub-anc
ENV HOME /home/ub-anc
WORKDIR $HOME

RUN cd $HOME \
    && curl -SL https://s3-us-west-2.amazonaws.com/qgroundcontrol/dependencies/Qt5.7.1-linux-min.tar.bz2 | tar -xj

ENV PATH=$HOME/Qt5.7-linux/5.7/gcc_64/bin:$PATH
ENV LD_LIBRARY_PATH=$HOME/Qt5.7-linux/5.7/gcc_64/lib:$LD_LIBRARY_PATH
ENV QML2_IMPORT_PATH=$HOME/Qt5.7-linux/5.7/gcc_64/qml:$QML2_IMPORT_PATH
ENV QT_PLUGIN_PATH=$HOME/Qt5.7-linux/5.7/gcc_64/plugins:$QT_PLUGIN_PATH

RUN cd $HOME \
    && curl -SL https://www.nsnam.org/release/ns-allinone-3.27.tar.bz2 | tar -xj \
    && cd ns-allinone-3.27/ns-3.27 \
    && ./waf configure --build-profile=release \
            --disable-examples --disable-tests --disable-python \
            --enable-modules=core,network,internet,wifi,mobility,olsr,aodv --prefix=$HOME/ns-3 \
    && ./waf build -j4 && ./waf install

ENV LD_LIBRARY_PATH=$HOME/ns-3/lib:$LD_LIBRARY_PATH

RUN cd $HOME \
    && git clone https://github.com/jmodares/UB-ANC-Emulator \
    && mkdir build-emulator \
    && cd build-emulator \
    && qmake ../UB-ANC-Emulator \
    && make -j4

RUN cd $HOME \
    && git clone https://github.com/ArduPilot/ardupilot \
    && cd ardupilot \
    && git checkout Copter-3.5 \
    && git submodule update --init modules/waf \
    && ./waf configure --board sitl --prefix=$HOME/sitl \
    && ./waf build -j4 --target=bin/arducopter && ./waf install --target=bin/arducopter

RUN cd $HOME \
    && git clone https://github.com/jmodares/follower \
    && mkdir build-follower \
    && cd build-follower \
    && qmake ../follower \
    && make -j4

RUN cd $HOME \
    && mkdir emulator emulator/mav \
    && cp build-emulator/engine/release/emulator emulator \
    && cp UB-ANC-Emulator/script/build_objects.sh emulator \
    && cp UB-ANC-Emulator/script/start_emulator.sh emulator \
    && cp sitl/bin/arducopter emulator/mav \
    && cp ardupilot/Tools/autotest/default_params/copter.parm emulator/mav \
    && cp build-follower/agent/release/agent emulator/mav

RUN cd $HOME \
    && cd build-emulator && make clean && cd .. \
    && cd build-follower && make clean && cd .. \
    && rm -rf ns-allinone-3.27 UB-ANC-Emulator ardupilot follower
