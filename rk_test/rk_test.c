#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "ecrt.h"
#include "Rockchip_MADHT1505BA1.h"

bool app_run = true;

void sigint_handler(int sig){
    if(sig == SIGINT){
        // ctrl+c退出时执行的代码
        printf("ctrl+c pressed!\n");
        app_run = false;
    	printf("rk_test end\n");
    	MADHT1505BA1_master_deinit(); 
    }
}

int main(int argc, char **argv) {
	printf("rk_test start\n");
	int ret = 0;
	int choice = 0;
	MADHT1505BA1_object slave0;

	signal(SIGINT, sigint_handler);

	ret = MADHT1505BA1_master_init();
	if(ret == -1) {
		printf("MADHT1505BA1_master_init is err\n");
	}
	slave0.alias = 0;
	slave0.position = 0;
	slave0.cpu_core = 3;
	ret = MADHT1505BA1_slaves_init(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slaves_init0 is err\n");
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
	ret = MADHT1505BA1_slave_start(&slave0);
	if(ret == -1) {
		printf("MADHT1505BA1_slave_start0 is err\n");
		return -1;
	}

	printf("Please wait while checking whether the motor is operational...\n");
	while((MADHT1505BA1_check_motor(&slave0) == -1)) {
		sleep(1);
	}
	printf("motor is ok\n");

	while(app_run) {
    	printf("1. Motor operation\n");
    	printf("2. Motor stop\n");
    	printf("3. exit\n");
    	printf("\nEnter your choice (1-3): ");
    	scanf("%d", &choice);

    	switch(choice) {
    	    case 1:
    	        MADHT1505BA1_motor_start(&slave0);
    	        break;
    	        
    	    case 2:
    	        MADHT1505BA1_motor_stop(&slave0);
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