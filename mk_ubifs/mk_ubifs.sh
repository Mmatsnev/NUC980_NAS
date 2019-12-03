#!/bin/bash
echo "******************************mkfs.ubifs*********************************"
rm *.img
mkfs.ubifs -F -x lzo -m 2048 -e 126976 -c 732 -o rootfs_ubifs.img -d ../rootfs 
ubinize -o ubi.img -m 2048 -p 131072 -O 2048 -s 2048 ubinize.cfg 
