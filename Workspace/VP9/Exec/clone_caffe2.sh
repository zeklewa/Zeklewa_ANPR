#!/bin/bash

git clone --recursive https://github.com/caffe2/caffe2.git && cd caffe2

# This will build Caffe2 in an isolated directory so that Caffe2 source is
# unaffected
mkdir build && cd build

# This configures the build and finds which libraries it will include in the
# Caffe2 installation. The output of this command is very helpful in debugging
cmake ..

# This actually builds and installs Caffe2 from makefiles generated from the
# above configuration step
sudo make install
