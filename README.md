Copyright © 2015 - 2018 Jalil Modares

This program was part of my Ph.D. Dissertation research in the Department of Electrical Engineering at the University at Buffalo. I worked in UB's Multimedia Communications and Systems Laboratory with my Ph.D. adviser, [Prof. Nicholas Mastronarde](http://www.eng.buffalo.edu/~nmastron).

If you use this program for your work/research, please cite:
[J. Modares, N. Mastronarde, K. Dantu, "UB-ANC Emulator: An Emulation Framework for Multi-agent Drone Networks"](https://doi.org/10.1109/SIMPAR.2016.7862404).

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

# UB-ANC Emulator
## An Emulation Framework for Multi-Agent Drone Networks
The UB-ANC Emulator is an emulation environment created to design, implement, and test various applications (missions) involving one or more drones in software, and provide seamless transition to experimentation. It provides flexibility in terms of the underlying flight dynamics and network simulation models. By default, it provides low-fidelity flight dynamics and network simulation, thus high scalability (it can support a large number of emulated agents). Depending on the application, it can connect to a high-fidelity physics engine for more accurate flight dynamics of agents (drones). It can also connect to a high-fidelity network simulation to model the effect of interference, packet losses, and protocols on network throughput, latency, and reliability (e.g., we have integrated [ns-3](https://www.nsnam.org) into the emulator). Another important aspect of the UB-ANC Emulator is its ability to be extended to different setups and connect to external communication hardware. This capability allows robotics researchers to emulate the mission planning part in software while the network researcher tests new network protocols on real hardware, or allows a network of real drones to connect to emulated drones and coordinate their tasks.

## Build

The current version of the UB-ANC Emulator uses [QGroundControl 3.2](http://qgroundcontrol.com) and [ns-3.27](https://www.nsnam.org) as its main libraries.
The build process explained here is targeted for Linux (Debian) platforms. We recommend using [Ubuntu 16.04](http://releases.ubuntu.com/16.04/). If you would like to use the emulator on other platforms, such as Windows, we have also provided a docker image. Please read the [Docker](#docker) section for more details.

The following packages need to be installed before building the emulator:

```
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install build-essential \
    libgl1-mesa-dev libsdl2-dev python-future \
    libfontconfig1 dbus-x11 geoclue curl git netcat xvfb
```

Then, we use `build_emulator.sh` to build and setup the emulator.

```
cd ~
mkdir ub-anc && cd ub-anc
curl -sSL \
    https://raw.githubusercontent.com/jmodares/UB-ANC-Emulator/master/script/build_emulator.sh \
    | bash
```

The build process takes some time. After the build is finished, you will have a new directory **~/ub-anc/emulator/** that has everything needed to run the emulator.

Finally, you need to run the following command so you can access the serial port:

```
sudo usermod -a -G dialout $USER
```

> IMPORTANT: You need to log out and log in again before you can run the emulator.


## Configuration
The UB-ANC Emulator uses the **objects** directory to recognize agents (drones) in the system. Every sub-directory in the **objects** directory represents an emulated agent. These sub-directories are named with the format **mav_xxx**, where *mav* stands for *micro air vehicle* and *xxx* denotes the MAV ID in the range **001 - 250**. There are three important files in each **mav_xxx** directory:

* **agent:** The mission executable

* **arducopter:** The firmware executable emulating the flight controller

* **copter.parm:** The default parameters used in the firmware

Together, the above three files define the agent.

In many cases, you will want all of the agents to operate with the same mission executable, firmware executable, and firmware parameters. We have created the script `build_objects.sh` to help with this. By running `build_objects.sh n`, where `n` is the number of agents you wish to emulate, the sub-directories **mav_001**, **mav_002**, ..., **mav_n** will be created in the **objects** directory and populated with all three default files from the **mav** directory. For instance, the following will create 10 replicas of the drone that is defined in the **mav** directory:

```
cd ~/ub-anc/emulator
./setup_objects.sh 10
```

By default, the agent executable is the [follower](https://github.com/jmodares/follower) mission. To build and test your own missions, we recommend starting from the [UB-ANC Agent](https://github.com/jmodares/UB-ANC-Agent) template mission. After compiling the UB-ANC Agent mission (or any other mission), you simply put the resulting executable file in the emulator's **mav** directory, run the `build_objects.sh` script, and then run the **emulator**.

## Run
To run the emulator, use the script `start_emulator.sh`. This script starts all of the agents' firmwares, waits for the emulator to connect to all of the firmwares, and then starts all of the corresponding agents so that they can connect to the emulator.

```
cd ~/ub-anc/emulator
./start_emulator.sh
```

Note that you cannot start the mission until you receive the following messages from the drones (which are accessible by clicking on the [Vehicle Messages](https://docs.qgroundcontrol.com/en/toolbar/toolbar.html) status icon in QGroundControl):

```
[XXX] Info: EKF2 IMU0 is using GPS
[XXX] Info: EKF2 IMU1 is using GPS
```
> IMPORTANT: For more details on running the follower mission [click here](https://github.com/jmodares/follower)

## Advanced users

### QGroundControl and ns-3 options
All options that are available in [QGroundControl](https://dev.qgroundcontrol.com/en/command_line_options.html) and [ns-3](https://www.nsnam.org/docs/tutorial/html/tweaking.html) are also available in the emulator. These can be set in the *start_emulator* function in `start_emulator.sh`. For instance, you can utilize the logging capabilities of [ns-3](https://www.nsnam.org/docs/manual/html/logging.html). You can also run the emulator without the GUI:

```
cd ~/ub-anc/emulator
./start_emulator.sh -c
```

By default, the emulator is configured to start with the **AODV** routing protocol when it runs in console mode (see the *start_emulator* function in `start_emulator.sh`). You can change or add more options if you need to, e.g., *RxGain, Reception gain (dB)* and see their effect on the mission.

> IMPORTANT: Port **10 * i + 5760** can be used to connect to agent (drone) **i**.

### Output traces and logging
By default, `start_emulator.sh` suppresses all messages from the agents by piping their outputs to **/dev/null**. This eliminates the overheads associated with logging and writing to the console, which allows the emulator to run with more agents. In many situations, however, it is useful to view information in the console in real-time and to log information for later analysis. You can modify `start_emulator.sh` to enable this functionality.


## Docker
There is a public Docker image with UB-ANC Emulator installed which can be loaded and used. First you need to install [Docker](https://docs.docker.com/engine/installation), and setup its [privilege access](https://docs.docker.com/engine/installation/linux/linux-postinstall/). You can also build the Docker image locally:

```
docker build -t jmod/ub-anc-emulator:latest \
    https://raw.githubusercontent.com/jmodares/UB-ANC-Emulator/master/script/Dockerfile
```

If you don't build the image locally, Docker will download it automatically. On Linux platforms you can run Docker container and connect it to X server on the host so that you can run the emulator in the container with GUI:

```
xauth nlist $DISPLAY \
    | sed -e 's/^..../ffff/' \
    | xauth -f /tmp/.docker.xauth nmerge -
docker run -it \
    --env DISPLAY=$DISPLAY \
    --env XAUTHORITY=/tmp/.docker.xauth \
    --volume /tmp/.X11-unix:/tmp/.X11-unix \
    --volume /tmp/.docker.xauth:/tmp/.docker.xauth \
    --device /dev/dri:/dev/dri \
    --device /dev/snd:/dev/snd \
    --volume $PWD/docker:/tmp/emulator \
    --publish 5770:5770 \
    --publish 5780:5780 \
    jmod/ub-anc-emulator:latest
```

On other platforms though, you need to find a way to map the X server socket between container and host, like [Xming](https://sourceforge.net/projects/xming/) for Windows. A better solution is to run the container without mapping:

```
docker run -it \
    --volume $PWD/docker:/tmp/emulator \
    --publish 5770:5770 \
    --publish 5780:5780 \
    jmod/ub-anc-emulator:latest
```

Then you can start emulator in console mode, and use [QGroundControl](http://qgroundcontrol.com/downloads) on the host to connect to agent's port to visualize it in the system.

Two things need to be noticed:
* As you can see a new directory **docker** will be created on the host (in the current directory), and it will be mounted to **/tmp/emulator** directory on the container. This can be used to share files between the host and the container.
* In order to connect to a port in container, you need to publish it `--publish` to the host, as you can see in the previous commands we publish two ports so that we can connect to them with QGroundControl (running on the host).

After you login to the container, you can start by:

```
cp -r emulator/ /tmp/
cd /tmp/emulator
```

By copying the **emulator** to the **/tmp/emulator**, you populate the container's **emulator** directory into the **docker** directory on the host so that you have access to the files generated by emulator later.

You can also use **docker** dirctory to put the mission source code and then build the mission using container. Notice that the username and password is **ub-anc**.

> Check all scripts in the **script** directory for more information.
