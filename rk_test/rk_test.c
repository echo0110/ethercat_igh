#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <sched.h>
#include <pthread.h>
#include <stdarg.h>
#include "ecrt.h"
#include "Rockchip_MADHT1505BA1.h"


bool app_run = true;
void sigint_handler(int sig){
    if(sig == SIGINT){
        // ctrl+c退出时执行的代码
        printf("ctrl+c pressed!\n");
        app_run = false;
    }
}

int main(int argc, char **argv) {
	printf("rk_test start\n");
	int ret = 0;
	signal(SIGINT, sigint_handler);
	MADHT1505BA1_object slave0;
	ret = MADHT1505BA1_master_init();
	if(ret == -1) {
		printf("MADHT1505BA1_master_init is err\n");
	}
	slave0.alias = 0;
	slave0.position = 0;
	slave0.cpu_core = 3;
	ret = MADHT1505BA1_slaves_init(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_init is err\n");
		return -1;
	}
	ret = MADHT1505BA1_master_activate();
	if(ret == -1) {
		printf("MADHT1505BA1_master_activate is err\n");
		return -1;
	}
	ret = MADHT1505BA1_slaves_activate(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_activate is err\n");
		return -1;
	}	
	ret = MADHT1505BA1_slave_start(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slave_start is err\n");
		return -1;
	}
	while(app_run) {
		usleep(100);
	}
	MADHT1505BA1_master_deinit();
	
	return 0;
}