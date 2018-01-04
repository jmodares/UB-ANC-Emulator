#!/bin/bash

#DISPLAY=:0
XSOCK=/tmp/.X11-unix
XAUTH=/tmp/.docker.xauth

STL_PORT=5760

if [ $# -eq 0 ]; then
    echo "Usage: cmd n"
    exit 1
fi

num_objs=$1

function start_docker {
    if [ ! -d "docker" ]; then
        mkdir docker
    fi

#    xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -

    ports=""
    for i in $(seq 1 $num_objs) ; do
        ports=$ports"--publish $((10 * i + STL_PORT)):$((10 * i + STL_PORT)) "
    done

    docker run -it \
        --env XAUTHORITY=$XAUTH \
        --env DISPLAY=$DISPLAY \
        --volume $PWD/docker:/tmp/emulator \
        --volume $XSOCK:$XSOCK \
        --volume $XAUTH:$XAUTH \
        --device /dev/dri:/dev/dri \
        --device /dev/snd:/dev/snd \
        $ports \
        jmod/ub-anc-emulator:latest
}

start_docker
