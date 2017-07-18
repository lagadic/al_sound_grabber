# al_sound_grabber

Ubuntu 14.04 : you can use the toolchain 2.4
Ubuntu 16.04 : you have to use the toolchain 2.5 (2.4 not working). Working with Pepper OS 2.4.

## Installation
* Clone `al_sound_grabber` in your qibuild workspace:

        $ git clone https://github.com/lagadic/al_sound_grabber.git

* Configure project:   

        $ configure --release -c toolchain_2_4

* Go in the build folder and `make`

        $ cd build-toolchain_2_4
        $ make -j4

## Usage

    $ cd ~/romeo/cpp/workspace/romeo_sound_processing/build-toolchain_2_4-release
    $ ./sdk/bin/al_sound_grabber --pip $PEPPER_IP --pport 9559


## TODO:
* Build with crosstoolchain to run on the robot
