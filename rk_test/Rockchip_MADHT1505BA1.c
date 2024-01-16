#define _GNU_SOURCE
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

#define FREQUENCY 1000
#define CLOCK_TO_USE CLOCK_MONOTONIC
#define MEASURE_TIMING
#define TARGET_VELOCITY        1124000 /*target velocity*/

#define NSEC_PER_SEC (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)                                                   
#define SHIFT_NS  (NSEC_PER_SEC / FREQUENCY /4)

#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + \
        (B).tv_nsec - (A).tv_nsec)

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

int debug_mode = 0;
bool run = true;
const struct timespec cycletime = {0, PERIOD_NS};

// struct itimerval tv;
// struct sigaction sa;
// static unsigned int sig_alarms = 0;
//static unsigned int sync_ref_counter = 0;

static void printf_debug(const char* fmt, ...){
	if (debug_mode == 1) {
	  va_list args;
	  va_start(args, fmt); 
	  vprintf(fmt, args); 
	  va_end(args);
	}
}

static int thread_bind_cpu(int target_cpu)
{
    cpu_set_t mask;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    int i;

    if (target_cpu >= cpu_num)
        return -1;

    CPU_ZERO(&mask);
    CPU_SET(target_cpu, &mask);

    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
        perror("pthread_setaffinity_np");

    if (pthread_getaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
        perror("pthread_getaffinity_np");

    printf("Thread(%ld) bound to cpu:", gettid());
    for (i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &mask)) {
            printf(" %d", i);
            break;    
        }
    }
    printf("\n");

    return i >= cpu_num ? -1 : i;
}

static struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC) {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
    } else {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}

static int domain_regs_fill_in(MADHT1505BA1_object *object) {

	// for (int i = 0; i < ARRAY_SIZE(code); i++) {
	// 	object.domain_regs[i] = 
	// 		(ec_pdo_entry_reg_t){object.alias, object.position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, code[i], 0, (uint8_t *)object + sizeof(unsigned int) * i};
	// }

	object->domain_regs[0] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x6040, 0, &(object->control_word)};
	object->domain_regs[1] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x6060, 0, &(object->modes_of_operation)};
	object->domain_regs[2] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x607a, 0, &(object->target_position)};
	object->domain_regs[3] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x60b8, 0, &(object->touch_probe_function)};
	object->domain_regs[4] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x603f, 0, &(object->error_code)};
	object->domain_regs[5] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x6041, 0, &(object->status_word)};
	object->domain_regs[6] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x6061, 0, &(object->modes_of_operation_display)};
	object->domain_regs[7] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x6064, 0, &(object->position_actual_value)};
	object->domain_regs[8] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x60b9, 0, &(object->touch_probe_status)};
	object->domain_regs[9] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x60ba, 0, &(object->touch_probe_pos1_pos_value)};
	object->domain_regs[10] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x60f4, 0, &(object->following_error_actual_value)};
	object->domain_regs[11] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x60fd, 0, &(object->digital_inputs)};
	object->domain_regs[12] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x606c, 0, &(object->current_velocity)};
	object->domain_regs[13] = (ec_pdo_entry_reg_t){object->alias, object->position, MADHT1505BA1_vendor, MADHT1505BA1_product_code, 0x60ff, 0, &(object->target_velocity)};
	object->domain_regs[14] = (ec_pdo_entry_reg_t){};

    object->domain_pd = NULL;
    object->user_velocity = 0;
    object->change_velocity = false;
	
    return 0;
}

static void check_master_state(void)
{

    ec_master_state_t ms;
 
    ecrt_master_state(master, &ms);
 
    if (ms.slaves_responding != master_state.slaves_responding) {
        printf_debug("found %u slave(s).\n", ms.slaves_responding);
    }
    if (ms.al_states != master_state.al_states) {
        printf_debug("AL states: 0x%02X.\n", ms.al_states);
    }
    if (ms.link_up != master_state.link_up) {
        printf_debug("Link is %s.\n", ms.link_up ? "up" : "down");
    }
 
    master_state = ms;
}

static void check_slave_config_states(MADHT1505BA1_object *object)
{

    ec_slave_config_state_t s;
 
    ecrt_slave_config_state(object->sc, &s);
 
    if (s.al_state != object->sc_state.al_state) {
        printf_debug("slaveDrive %d: State 0x%02X.\n", object->alias, s.al_state);
    }
    if (s.online != object->sc_state.online) {
        printf_debug("slaveDrive %d: %s.\n", object->alias, s.online ? "online" : "offline");
    }
    printf_debug("slaveDrive %d: %s  operational.\n", object->alias, s.operational ? "yes" : "Not ");
 
    object->sc_state = s;
}

static void check_domain_state(MADHT1505BA1_object *object)
{

    ec_domain_state_t ds;
 
    ecrt_domain_state(object->domain, &ds);
 
    if (ds.working_counter != object->domain_state.working_counter) {
        printf_debug("Domain %d: WC %u.\n", object->alias, ds.working_counter);
    }
    if (ds.wc_state != object->domain_state.wc_state) {
        printf_debug("Domain %d: State %u.\n", object->alias, ds.wc_state);
    }
 
    object->domain_state = ds;
}

int MADHT1505BA1_master_init(void) {
	char* tmp = NULL;
	printf("rockchip MADHT1505BA1 Motor drive program\n");

	tmp = getenv("RKOCKCHIP_MADHT1505BA1_DEBUG");
    if (tmp == NULL) {
        printf("env RKOCKCHIP_MADHT1505BA1_DEBUG not set\n");
        debug_mode = 0;
    }else if (!strcmp("1", tmp)) {
		debug_mode = 1;
	}

	master = ecrt_request_master(0);
    if (!master) {
		printf("ecrt_request_master is err\n");
		return -1;
	}
	printf("request_master sucess, check slaves responding %d\n", master_state.slaves_responding);
	check_master_state();

	return 0;
}

int MADHT1505BA1_slaves_init(MADHT1505BA1_object *object) {
	domain_regs_fill_in(object);
	object->sc = ecrt_master_slave_config(
								master,
								object->alias,
								object->position,
								MADHT1505BA1_vendor,
								MADHT1505BA1_product_code);
    if(!object->sc) {
        printf("Failed to get slave configuration.\n");
        return -1;
    }
	check_slave_config_states(object);
	
    object->domain = ecrt_master_create_domain(master);
    if (!object->domain) {
        printf("ecrt_master_create_domain is fail\n");
        return -1;  
    }
    printf("Configuring PDOs...\n");
    if (ecrt_slave_config_pdos(object->sc, EC_END, slave_0_syncs)) {
        printf("Failed to configure PDOs.\n");
        return -1;
    }
    // for(int i = 0; i<15; i++) {
    // 	printf("domain_regs[%d] : alias = %d position = %d index = %x\n",i, object->domain_regs[i].alias, object->domain_regs[i].position, object->domain_regs[i].index);
    // }
    if(ecrt_domain_reg_pdo_entry_list(object->domain, object->domain_regs)) {
        printf("Failed to ecrt_domain_reg_pdo_entry_list.\n");
        return -1;
    };

    ecrt_slave_config_dc(object->sc, 0x300, PERIOD_NS, 0, 0, 0);
	return 0;
}

int MADHT1505BA1_master_activate(void) {
    printf("Activating master...\n");
    if (ecrt_master_activate(master)) {
        printf("ecrt_master_activate is fail\n");
        return -1;
    }
    return 0;
}

int MADHT1505BA1_slaves_activate(MADHT1505BA1_object *object) {
    printf("activate slaves %d\n", object->alias);
    if (!(object->domain_pd = ecrt_domain_data(object->domain))) {
        printf("ecrt_domain_data is fail\n");
        return -1;
    }else {
        printf("activate slaves %d is success\n", object->alias);
        return 0;
    }
}

int MADHT1505BA1_master_deinit(void) {
    run = false;
    ecrt_master_deactivate(master);
    ecrt_release_master(master);
    master = NULL;
    printf("MADHT1505BA1_master_deinit\n");
}

void *slave_pthread(void *arg) {
    struct timespec wakeupTime, time;
	MADHT1505BA1_object *object = (MADHT1505BA1_object *)arg;
	int counter = 0;
    uint16_t    status;
    int8_t      opmode;
    int32_t     cur_velocity;
	struct sched_param param;
    int maxpri, count;

    printf("slave %d bind_cpu\n", object->alias);
    if(thread_bind_cpu(object->cpu_core) == -1) {
        printf("bind cpu core fail\n");
    }

    // The scheduling priority is the highest
    printf("slave %d sched_get_priority_max\n", object->alias);
    maxpri = sched_get_priority_max(SCHED_FIFO);
    if(maxpri == -1) { 
        printf("sched_get_priority_max() failed");
    }

    printf("slave %d max priority of SCHED_FIFO is %d\n", object->alias, maxpri);
    param.sched_priority = maxpri;
    if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) { 
        perror("sched_setscheduler() failed");
    }
    printf("end thread set\n");

    clock_gettime(CLOCK_TO_USE, &wakeupTime);
	while(run) {
		
        wakeupTime = timespec_add(wakeupTime, cycletime);
		clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wakeupTime, NULL);

        // Write application time to master
        //
        // It is a good idea to use the target time (not the measured time) as
        // application time, because it is more stable.
        //
        ecrt_master_application_time(master, TIMESPEC2NS(wakeupTime));
        
        /*Receive process data*/
        ecrt_master_receive(master);
        ecrt_domain_process(object->domain);
        // check process data state (optional)
       	check_domain_state(object);
       	if(counter) {
       		counter--;
       	}else {
       		counter = FREQUENCY;
       		check_master_state();
       		check_slave_config_states(object);
       		EC_WRITE_U16(object->domain_pd + object->control_word, 0x80);
       		status = EC_READ_U16(object->domain_pd + object->status_word);
       		opmode = EC_READ_U8(object->domain_pd + object->modes_of_operation_display);
       		cur_velocity = EC_READ_S32(object->domain_pd + object->current_velocity);
       		printf_debug("slave %d madht:  act velocity = %d ,  status = 0x%x, opmode = 0x%x\n", object->alias, cur_velocity,  status, opmode);
           	if( (status & 0x004f) == 0x0040) {
               	printf_debug("0x06\n");
               	EC_WRITE_U16(object->domain_pd + object->control_word, 0x0006);
               	EC_WRITE_U8(object->domain_pd + object->modes_of_operation, 9);
           	}
           	else if( (status & 0x006f) == 0x0021) {
               	printf_debug("0x07\n");
               	EC_WRITE_U16(object->domain_pd + object->control_word, 0x0007);
           	}
           	else if( (status & 0x006f) == 0x0023) {
               	printf_debug("0x0f\n");
               	EC_WRITE_U16(object->domain_pd + object->control_word, 0x000f);
               	EC_WRITE_S32(object->domain_pd + object->target_velocity, object->user_velocity);
           	}
           	//operation enabled
           	else if( (status & 0x006f) == 0x0027) {
               	printf_debug("0x1f\n");
               	EC_WRITE_U16(object->domain_pd + object->control_word, 0x001f);
           	}
           	if(object->change_velocity) {
               	printf_debug("change velocity\n");
               	EC_WRITE_U16(object->domain_pd + object->control_word, 0x0007); // stop slaves
               	object->change_velocity = false;
           	}
           	
       	}

        clock_gettime(CLOCK_TO_USE, &time);
        ecrt_master_sync_reference_clock_to(master, TIMESPEC2NS(time));

        ecrt_master_sync_slave_clocks(master);
        // send process data
        ecrt_domain_queue(object->domain);
        ecrt_master_send(master);
	}
}

int MADHT1505BA1_slave_start(MADHT1505BA1_object *object) {
	int err;
    printf("MADHT1505BA1_slave_start: %d\n", object->alias);
	err = pthread_create(&object->thread, NULL, slave_pthread, object);
	if(err != 0) {
		printf("MADHT1505BA1_drive_slave: can't create thread\n");
		return -1;
	}else {
		return 0;
	}
}

int MADHT1505BA1_motor_start(MADHT1505BA1_object *object) {
    uint16_t    status;
    status = EC_READ_U16(object->domain_pd + object->status_word);
    if(status == 0x1237) {
        object->change_velocity = true;
        object->user_velocity = TARGET_VELOCITY;
    }else {
        printf("slave %d not start\n", object->alias);
    }
    return 0;
}

int MADHT1505BA1_motor_stop(MADHT1505BA1_object *object) {
    uint16_t    status;
    status = EC_READ_U16(object->domain_pd + object->status_word);
    if(status == 0x1237) {
        object->change_velocity = true;
        object->user_velocity = 0;
    }else {
        printf("slave %d not start\n", object->alias);
    }
    return 0;
}

int MADHT1505BA1_check_motor(MADHT1505BA1_object *object) {
    uint16_t    status;
    status = EC_READ_U16(object->domain_pd + object->status_word);
    if(status != 0x1237) {
        return -1;
    }else {
        return 1;
    }
}