#!/bin/bash

if [ $# -eq 0 ]; then
    BASEDIR=$PWD
else
    BASEDIR=$1
fi

function setup_qt {
    cd $BASEDIR
    curl -SL https://raw.githubusercontent.com/fghanei/UB-ANC-Dependencies/master/Qt5.7.1-linux-min.tar.bz2 | tar -xj

    echo export PATH=$BASEDIR/Qt5.7-linux/5.7/gcc_64/bin:\$PATH >> $BASEDIR/setup_emulator.sh
    echo export LD_LIBRARY_PATH=$BASEDIR/Qt5.7-linux/5.7/gcc_64/lib:\$LD_LIBRARY_PATH >> $BASEDIR/setup_emulator.sh
    echo export QML2_IMPORT_PATH=$BASEDIR/Qt5.7-linux/5.7/gcc_64/qml:\$QML2_IMPORT_PATH >> $BASEDIR/setup_emulator.sh
    echo export QT_PLUGIN_PATH=$BASEDIR/Qt5.7-linux/5.7/gcc_64/plugins:\$QT_PLUGIN_PATH >> $BASEDIR/setup_emulator.sh
    source $BASEDIR/setup_emulator.sh
}

function build_ns3 {
    cd $BASEDIR
    curl -SL https://raw.githubusercontent.com/fghanei/UB-ANC-Dependencies/master/ns-allinone-3.27.tar.bz2 | tar -xj
    cd ns-allinone-3.27/ns-3.27
    ./waf configure --build-profile=release \
            --disable-examples --disable-tests --disable-python \
            --enable-modules=core,network,internet,wifi,mobility,olsr,aodv --prefix=$BASEDIR/ns-3
    ./waf build -j4
    ./waf install

    echo export LD_LIBRARY_PATH=$BASEDIR/ns-3/lib:\$LD_LIBRARY_PATH >> $BASEDIR/setup_emulator.sh
    source $BASEDIR/setup_emulator.sh
}

function build_emulator {
    cd $BASEDIR
    git clone https://github.com/jmodares/UB-ANC-Emulator
    mkdir build-emulator
    cd build-emulator
    qmake ../UB-ANC-Emulator
    make -j4
}

function build_firmware {
    cd $BASEDIR
    git clone https://github.com/ArduPilot/ardupilot
    cd ardupilot
    git checkout Copter-3.5
    git submodule update --init modules/waf
    ./waf configure --board sitl --prefix=$BASEDIR/sitl
    ./waf build -j4 --target=bin/arducopter
    ./waf install --target=bin/arducopter
}

function build_mission {
    cd $BASEDIR
    git clone https://github.com/jmodares/follower
    mkdir build-follower
    cd build-follower
    qmake ../follower
    make -j4
}

function setup_emulator {
    cd $BASEDIR
    mkdir emulator emulator/mav
    cp build-emulator/engine/release/emulator emulator
    cp UB-ANC-Emulator/script/setup_objects.sh emulator
    cp UB-ANC-Emulator/script/start_emulator.sh emulator
    cp sitl/bin/arducopter emulator/mav
    cp ardupilot/Tools/autotest/default_params/copter.parm emulator/mav
    cp build-follower/agent/release/agent emulator/mav
    cp $BASEDIR/setup_emulator.sh emulator
}

function clean_up {
    cd $BASEDIR
    cd build-emulator
    make clean
    cd ..
    cd build-follower
    make clean
    cd ..
    rm -rf ns-allinone-3.27 UB-ANC-Emulator ardupilot follower setup_emulator.sh

    exit
}

trap clean_up SIGHUP SIGINT SIGTERM

setup_qt
build_ns3
build_emulator
build_firmware
build_mission
setup_emulator
clean_up
