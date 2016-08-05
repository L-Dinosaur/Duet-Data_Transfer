#! /bin/bash
sudo modprobe duet
sudo duet status start
sudo ./duet_collect -f 20 -d 200 -o -e -g -p /var
