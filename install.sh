#!/bin/bash

mkdir build && cd build
cmake .. && sudo make install
sudo cp devel/lib/lib* /usr/local/lib
sudo ldconfig