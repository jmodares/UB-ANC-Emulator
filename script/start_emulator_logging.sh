#!/bin/bash
#

HOME_LAT=43.000755
HOME_LON=-78.776023

STEP_LAT=0.00008

AGENT_FILE="agent"
FIRMWARE_FILE="arducopter"

STL_PORT=5760
NET_PORT=15760

MAV_DIR="mav_"
OBJ_DIR="objects"

qgc_pid=0
tail_pid=0

set -e

cli=0

DATE_STRING=$(date +"%Y-%m-%d-%H-%M-%S")

while getopts ":c" opt; do
    case ${opt} in
    c )
        cli=1
        ;;
    \? )
        echo "Usage: cmd [-c]"
        exit 1
        ;;
    esac
done

function start_firmwares {
    for name in $(ls $OBJ_DIR) ; do
        mav_num=${name#$MAV_DIR}
        mav_num=$((10#$mav_num))

        cd $OBJ_DIR/$name
        ./$FIRMWARE_FILE --speedup 1 \
            --instance $mav_num \
            --param SYSID_THISMAV=$mav_num \
            --synthetic-clock \
            --home $(echo $HOME_LAT $mav_num $STEP_LAT | awk '{printf "%f", $1 + $2 * $3}'),$HOME_LON,0,0 \
            --model + \
            --defaults copter.parm \
            --uartA tcp:0 &> "log_firmware_$DATE_STRING.log" &
        cd $OLDPWD
    done

    for name in $(ls $OBJ_DIR) ; do
        mav_num=${name#$MAV_DIR}
        mav_num=$((10#$mav_num))

        while ! nc -z localhost $((10 * mav_num + STL_PORT + 3)); do sleep 0.1; done
    done
}

function start_emulator {
    rm -rf /tmp/*.mavlink

    cd $OBJ_DIR/..
    export NS_LOG="WifiMac:WifiNetDevice:WifiPhy:YansErrorRateModel:YansWifiChannel:YansWifiPhy"

    if [ $cli -eq 1 ]; then
        xvfb-run --server-args="-screen 0 1024x768x24" ./emulator --clear-settings \
            --routing=aodv \
            --tracing=true \
            --PrintGroup=Wifi \
            --PrintAttributes=ns3::WifiPhy \
            --ChecksumEnabled=false \
            --ns3::YansWifiChannel::PropagationDelayModel=ns3::ConstantSpeedPropagationDelayModel \
            --ns3::YansWifiChannel::PropagationLossModel=ns3::FriisPropagationLossModel \
            --ns3::WifiPhy::RxGain=-10 \
            --ns3::WifiRemoteStationManager::NonUnicastMode=DsssRate1Mbps \
            --ns3::ConstantRateWifiManager::DataMode=DsssRate1Mbps \
            --ns3::ConstantRateWifiManager::ControlMode=DsssRate1Mbps &

    else
        ./emulator --clear-settings \
            --ChecksumEnabled=false \
            --ns3::YansWifiChannel::PropagationDelayModel=ns3::ConstantSpeedPropagationDelayModel \
            --ns3::YansWifiChannel::PropagationLossModel=ns3::FriisPropagationLossModel \
            --ns3::WifiPhy::RxGain=-10 \
            --ns3::WifiRemoteStationManager::NonUnicastMode=DsssRate1Mbps \
            --ns3::ConstantRateWifiManager::DataMode=DsssRate1Mbps \
            --ns3::ConstantRateWifiManager::ControlMode=DsssRate1Mbps &
    fi

    qgc_pid=$!
    cd $OLDPWD

    for name in $(ls $OBJ_DIR) ; do
        mav_num=${name#$MAV_DIR}
        mav_num=$((10#$mav_num))

        while ! nc -z localhost $((10 * mav_num + NET_PORT)); do sleep 0.1; done
    done
}

function start_agents {
    for name in $(ls $OBJ_DIR) ; do
        mav_num=${name#$MAV_DIR}
        mav_num=$((10#$mav_num))

        cd $OBJ_DIR/$name
        echo "---" > "log_agent_$DATE_STRING.log"
        ./$AGENT_FILE -I $mav_num &> "log_agent_$DATE_STRING.log" &
        tail "log_agent_$DATE_STRING.log" -f &
        tail_pid=$!
        cd $OLDPWD
    done
}

function clean_up {
    rm -rf /tmp/*.mavlink

#    lsof -n -i4TCP:$((10 * mav_num + STL_PORT + 3)) | grep ESTABLISHED | awk '{ print $2 }' | xargs kill -SIGTERM &> /dev/null
    ps -o pid= -C $AGENT_FILE | xargs kill -SIGTERM # &> /dev/null
    kill -9 $tail_pid

    exit
}

trap clean_up SIGHUP SIGINT SIGTERM

source setup_emulator.sh
start_firmwares
start_emulator
start_agents

wait $qgc_pid
#echo "Press Enter to finish!"
#read

clean_up
