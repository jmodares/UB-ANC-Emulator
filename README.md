Copyright © 2015 - 2018 Jalil Modares

This program was part of my Ph.D. Dissertation research in the Department of Electrical Engineering at the University at Buffalo. I worked in UB's Multimedia Communications and Systems Laboratory with my Ph.D. adviser, [Prof. Nicholas Mastronarde](http://www.eng.buffalo.edu/~nmastron).

If you use this program for your work/research, please cite:
[J. Modares, N. Mastronarde, K. Dantu, "UB-ANC Emulator: An Emulation Framework for Multi-agent Drone Networks"](https://doi.org/10.1109/SIMPAR.2016.7862404).

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

# UB-ANC Emulator
## An Emulation Framework for Multi-Agent Drone Networks
UB-ANC Emulator is an emulation environment created to design, implement, and test various applications (missions) involving one or more drones in software, and provide seamless transition to experimentation. It provides flexibility in terms of the underlying flight dynamics and network simulation models. By default, it provides low-fidelity flight dynamics and network simulation, thus high scalability (it can support a large number of emulated agents). Depending on the application, it can connect to a high-fidelity physics engine for more accurate flight dynamics of agents (drones). It can also connect to a high-fidelity network simulation to model the effect of interference, packet losses, and protocols on network throughput, latency, and reliability (e.g., we have integrated [ns-3](https://www.nsnam.org) into the emulator). Another important aspect of the UB-ANC Emulator is its ability to be extended to different setups and connect to external communication hardware. This capability allows robotics researchers to emulate the mission planning part in software while the network researcher tests new network protocols on real hardware, or allows a network of real drones to connect to emulated drones and coordinate their tasks.

## Build
The current version of UB-ANC Emulator uses [QGroundControl 3.2](http://qgroundcontrol.com) and [ns-3.27](https://www.nsnam.org) as its main libraries.
The build process explained here is targeted for Linux platforms. In order to use the emulator on other platforms such as Windows, there is a docker image provided that can be used. Please read Docker section for more detail.
These packages need to be install first:

```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libgl1-mesa-dev libsdl2-dev python-future libfontconfig1 dbus-x11 geoclue curl git netcat xvfb
```

Then we can use `build_emulator.sh` to build and setup the emulator.

```
cd ~
mkdir ub-anc
cd ub-anc
curl -sSL https://raw.githubusercontent.com/jmodares/UB-ANC-Emulator/master/script/build_emulator.sh | bash
```

The build process may take sometime. After the build finished, it has made a new directory **emulator** which has everything needed to run the emulator.

> You need to log out and log in again to run the emulator properly.

## Configuration
UB-ANC Emulator uses **objects** directory to recognizes agents (drones) in the system. Every directory in the **objects** directory represents an emulated agent (drone). The naming format is **mav_xxx**, where xxx shows the ID, in the range **001 - 250**. In each **mav_xxx** directory there are three main files:
* **agent**
  * The mission executable

* **arducopter**
  * The firmware executable emulating the flight controller

* **copter.parm**
  * The default parameters used in firmware

This can be done by running `build_objects.sh n`, where `n` is the number of agents in the network.

```
cd ~/ub-anc/emulator
./build_objects.sh 10
```

To build the objects directory, the script uses all three default files from **mav** directory in **emulator**. By default the agent executable is the [follower](https://github.com/jmodares/follower) mission. To build and test different mission, you need to use [UB-ANC Agent](https://github.com/jmodares/UB-ANC-Agent), and put it in the **mav** directory.

## Run
To run the emulator, use `start_emulator.sh`. It first starts all firmwares, waits for emulator to connect to all firmwares, and then starts all coresponding agents so that they can connect to the emulator.

```
cd ~/ub-anc/emulator
./start_emulator.sh
```

Note that you can not start the mission untill you recive these messages from drones:

```
[XXX] Info: EKF2 IMU0 is using GPS
[XXX] Info: EKF2 IMU1 is using GPS
```

Notice that all options that available to [QGroundControl](https://dev.qgroundcontrol.com/en/command_line_options.html) and [ns-3](https://www.nsnam.org/docs/tutorial/html/tweaking.html) are also available in the emulator, and you can set them in the script in *start_emulator* function. You can also utilize the logging capibiliies of [ns-3](https://www.nsnam.org/docs/manual/html/logging.html).

You can also run the emulator without gui:

```
cd ~/ub-anc/emulator
./start_emulator.sh -c
```

As you can see in the `start_emulator.sh`, in *start_emulator* function, the emulator starts with **AODV** routing protocol when it runs in console mode. You can change or add more options if you need, especially you can change *RxGain, Reception gain (dB)* and see its effect during the mission.

> It should be noted that port **10 * i + 5760** can be used to connect to agent (drone) **i**. 

## Docker
There is a public docker image with UB-ANC Emulator installed and can be used:
First you need to install [docker](https://docs.docker.com/engine/installation) engine. and then run `start_docker.sh n` where `n` is the number of agents.

```
cd ~/ub-anc
curl -sSL https://raw.githubusercontent.com/jmodares/UB-ANC-Emulator/master/script/start_docker.sh -o start_docker.sh
chmod +x start_docker.sh
./start_docker.sh 10
```

After you login to the container, you can start by:

```
cp -r emulator /tmp
cd /tmp/emulator
./build_objects.sh 5
./start_emulator.sh
```

By copying the **emulator** to the **/tmp/emulator**, you populate the container's emulator into the **docker** directory in the host.

Note that as you can see the script is for Linux platforms. If you want to run it on other platforms, you need to map the XServer socket to the platform's descktop system, like [Xming](https://sourceforge.net/projects/xming/) for Windows. A better soplution is that you can start emulator in console mode, and install [QGroundControl](http://qgroundcontrol.com/downloads) and connect to specific port to visualize the agent (drone) in the system.

If you want to buld the docker locally, you can use the following:

```
docker build -t jmod/ub-anc-emulator:latest https://raw.githubusercontent.com/jmodares/UB-ANC-Emulator/master/script/Dockerfile
```

> Check the scripts `start_emulator.sh` and `start_docker.sh` for more information.
