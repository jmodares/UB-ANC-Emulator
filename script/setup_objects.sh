#!/bin/bash

FIRMWARE_FILE="arducopter"

STL_PORT=5760

MAV_DIR="mav_"
OBJ_DIR="objects"

num_objs=$1
mav_num=1

function setup_objects {
    if [ ! -d "mav" ]; then
        echo "Please provide mav directory first!"
        exit 1
    fi

    cd mav
    ./$FIRMWARE_FILE --wipe \
        --instance $mav_num \
        --model + \
        --defaults copter.parm \
        --uartA tcp:0 &> /dev/null &
    cd $OLDPWD

    while ! nc -z localhost $((10 * mav_num + STL_PORT + 3)); do sleep 0.1; done

    rm -rf mav/logs
    rm -rf $OBJ_DIR
    mkdir $OBJ_DIR

    for i in $(seq 1 $num_objs) ; do
        name=$(printf $MAV_DIR"%.3d" $i)

        cp -r mav $OBJ_DIR/$name
    done
}

#echo "Enter the number of MAV objects:"
#read num_objs

setup_objects
