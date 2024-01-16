#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "ecrt.h"
#include "Rockchip_MADHT1505BA1.h"

bool app_run = true;

int main(int argc, char **argv) {
	printf("rk_test start\n");
	int ret = 0;
	int choice = 0;
	MADHT1505BA1_object slave0;
	MADHT1505BA1_object slave1;
	ret = MADHT1505BA1_master_init();
	if(ret == -1) {
		printf("MADHT1505BA1_master_init is err\n");
	}
	slave0.alias = 0;
	slave0.position = 0;
	slave0.cpu_core = 3;
	slave1.alias = 1;
	slave1.position = 0;
	slave1.cpu_core = 3;
	ret = MADHT1505BA1_slaves_init(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_init0 is err\n");
		return -1;
	}
	ret = MADHT1505BA1_slaves_init(&slave1);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_init1 is err\n");
		return -1;
	}
	ret = MADHT1505BA1_master_activate();
	if(ret == -1) {
		printf("MADHT1505BA1_master_activate is err\n");
		return -1;
	}
	ret = MADHT1505BA1_slaves_activate(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_activate0 is err\n");
		return -1;
	}
	ret = MADHT1505BA1_slaves_activate(&slave1);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_activate1 is err\n");
		return -1;
	}	
	ret = MADHT1505BA1_slave_start(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slave_start0 is err\n");
		return -1;
	}
	ret = MADHT1505BA1_slave_start(&slave1);
	if(ret == -1) {
		printf("MADHT1505BA1_slave_start1 is err\n");
		return -1;
	}

	while(app_run) {
    	printf("1. Motor operation\n");
    	printf("2. Motor stop\n");
    	printf("3. exit\n");
    	printf("\nEnter your choice (1-3): ");
    	scanf("%d", &choice);

    	switch(choice) {
    	    case 1:
    	        MADHT1505BA1_motor_start(&slave0);
    	        MADHT1505BA1_motor_start(&slave1);
    	        break;
    	        
    	    case 2:
    	        MADHT1505BA1_motor_stop(&slave0);
    	        MADHT1505BA1_motor_stop(&slave1);
    	        break;
    	    
    	    case 3:
    	    	printf("rk_test end\n");
    	    	MADHT1505BA1_master_deinit();   	        
    	    	return 0;

    	    default:
    	        printf("Invalid choice!\n");
    	        break;
    	}
    	usleep(100);
	}
	
	return 0;
}