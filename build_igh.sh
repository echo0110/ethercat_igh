#!/bin/sh
#export  CROSS_COMPILE=/home/william/rongpin_rk3588_sdk/source/rk-linux5.10-SDK-20260122/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

#export ARCH=arm64


./configure \
--with-linux-dir=/home/william/rongpin_rk3588_sdk/source/rk-linux5.10-SDK-20260122/kernel \
--enable-stmmac \
--with-stmmac-kernel=5.10 \
--host=aarch64-none-linux-gnu \
#ARCH=arm64 \
#CROSS_COMPILE=aarch64-none-linux-gnu-
