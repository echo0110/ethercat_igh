export PATH=~/linux-develop/3568/buildroot/output/rockchip_rk3568/host/bin:$PATH && rm -rf rk_ec_test && aarch64-buildroot-linux-gnu-gcc ec_master_test_RK3568_MADHT1505BA1.c -I ../include/ -L ../lib/.libs  -lethercat -o rk_ec_test

aarch64-buildroot-linux-gnu-gcc Rockchip_MADHT1505BA1.c -I ../include/ -L ../lib/.libs  -lethercat -shared -fPIC -o libmadht1505ba1.so

aarch64-buildroot-linux-gnu-gcc rk_test.c  -I ../include/ -L ../lib/.libs -L ./ -lmadht1505ba1 -lethercat -o rk_test
