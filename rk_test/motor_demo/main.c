/*
 * Copyright (c) 2024 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#define _GNU_SOURCE
#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <pthread.h>

#include "hal_sdl.h"
#include "hal_drm.h"
#include "main.h"
#include "Rockchip_MADHT1505BA1.h"


static int g_indev_rotation = 0;
static int g_disp_rotation = LV_DISP_ROT_NONE;

static int quit = 0;

static int thread_bind_cpu(int target_cpu) {
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

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

int app_disp_rotation(void)
{
    return g_disp_rotation;
}

static void lvgl_init(void)
{
    lv_init();
#ifdef USE_SDL_GPU
    hal_sdl_init(0, 0, g_disp_rotation);
#else
    hal_drm_init(0, 0, g_disp_rotation);
#endif
    lv_port_fs_init();
    lv_port_indev_init(g_indev_rotation);
}

MADHT1505BA1_object slave0;
MADHT1505BA1_object slave1;

static int motor_init(void) {
    int ret;
    ret = MADHT1505BA1_master_init(3); //bind cpu core 3
    if(ret == -1) {
        printf("MADHT1505BA1_master_init is err\n");
        MADHT1505BA1_master_deinit();
    }
    slave0.alias = 0;
    slave0.position = 0;
    slave1.alias = 1;
    slave1.position = 0;
    
    ret = MADHT1505BA1_slaves_init(&slave0);
    if(ret == -1) {
        printf("MADHT1505BA1_slaves_init0 is err\n");
        MADHT1505BA1_master_deinit();
        return -1;
    }
    ret = MADHT1505BA1_slaves_init(&slave1);
    if(ret == -1) {
        printf("MADHT1505BA1_slaves_init1 is err\n");
        MADHT1505BA1_master_deinit();
        return -1;
    }
    ret = MADHT1505BA1_master_activate();
    if(ret == -1) {
        printf("MADHT1505BA1_master_activate is err\n");
        MADHT1505BA1_master_deinit();
        return -1;
    }
    ret = MADHT1505BA1_slaves_activate(&slave0);
    if(ret == -1) {
        printf("MADHT1505BA1_slaves_activate0 is err\n");
        MADHT1505BA1_master_deinit();
        return -1;
    }
    ret = MADHT1505BA1_slaves_activate(&slave1);
    if(ret == -1) {
        printf("MADHT1505BA1_slaves_activate1 is err\n");
        MADHT1505BA1_master_deinit();
        return -1;
    }
    
    ret = MADHT1505BA1_slave_start(2, &slave0, &slave1);
    if(ret == -1) {
        printf("MADHT1505BA1_slaves_activate1 is err\n");
        MADHT1505BA1_master_deinit();
        return -1;
    }


    printf("Please wait while checking whether the motor is operational...\n");
    while((MADHT1505BA1_check_motor(&slave0) == -1) || (MADHT1505BA1_check_motor(&slave1) == -1)) {
        sleep(1);
    }
    printf("motor is ok\n");
    return 0;
}

static void button0_run_event_handler(lv_obj_t * obj, lv_event_t event) {
    printf("button 0 run Clicked\n");
    MADHT1505BA1_motor_start(&slave0);
}

static void button1_run_event_handler(lv_obj_t * obj, lv_event_t event) {
    printf("button 1 run Clicked\n");
    MADHT1505BA1_motor_start(&slave1);
}

static void button0_stop_event_handler(lv_obj_t * obj, lv_event_t event) {
    printf("button 0 stop Clicked\n");
    MADHT1505BA1_motor_stop(&slave0);
}

static void button1_stop_event_handler(lv_obj_t * obj, lv_event_t event) {
    printf("button 1 stop Clicked\n");
    MADHT1505BA1_motor_stop(&slave1);
}

lv_obj_t * label0;
lv_obj_t * label1;
lv_obj_t * label2;
lv_obj_t * label3;

lv_obj_t * label0_status;
lv_obj_t * label0_speed;
lv_obj_t * label0_jitter;
lv_obj_t * label1_status;
lv_obj_t * label1_speed;
lv_obj_t * label1_jitter;


lv_obj_t * btn0_run;
lv_obj_t * btn1_run;
lv_obj_t * btn0_stop;
lv_obj_t * btn1_stop;

static void ui_init(void) {

        btn0_run = lv_btn_create(lv_scr_act());
        lv_obj_set_width(btn0_run, 270);
        lv_obj_set_height(btn0_run, 135);
        lv_obj_add_event_cb(btn0_run, button0_run_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(btn0_run, LV_ALIGN_CENTER, -300, -600);
        label0 = lv_label_create(btn0_run);
        lv_obj_align(label0, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label0, "motor 0 run");

        btn0_stop = lv_btn_create(lv_scr_act());
        lv_obj_set_width(btn0_stop, 270);
        lv_obj_set_height(btn0_stop, 135);
        lv_obj_add_event_cb(btn0_stop, button0_stop_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(btn0_stop, LV_ALIGN_CENTER, -300, -300);
        label1 = lv_label_create(btn0_stop);
        lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label1, "motor 0 stop");

        btn1_run = lv_btn_create(lv_scr_act());
        lv_obj_set_width(btn1_run, 270);
        lv_obj_set_height(btn1_run, 135);
        lv_obj_add_event_cb(btn1_run, button1_run_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(btn1_run, LV_ALIGN_CENTER, -300, 300);
        label2 = lv_label_create(btn1_run);
        lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label2, "motor 1 run");

        btn1_stop = lv_btn_create(lv_scr_act());
        lv_obj_set_width(btn1_stop, 270);
        lv_obj_set_height(btn1_stop, 135);
        lv_obj_add_event_cb(btn1_stop, button1_stop_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(btn1_stop, LV_ALIGN_CENTER, -300, 600);
        label3 = lv_label_create(btn1_stop);
        lv_obj_align(label3, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label3, "motor 1 stop");

        label0_status = lv_label_create(lv_scr_act());
        lv_obj_align(label0_status, LV_ALIGN_CENTER, 100, -600);
        lv_label_set_text(label0_status, "motor 0 status :");
        label0_speed = lv_label_create(lv_scr_act());
        lv_obj_align(label0_speed, LV_ALIGN_CENTER, 100, -450);
        lv_label_set_text(label0_speed, "motor 0 speed :");
        label0_jitter = lv_label_create(lv_scr_act());
        lv_obj_align(label0_jitter, LV_ALIGN_CENTER, 100, -300);
        label1_status = lv_label_create(lv_scr_act());
        lv_obj_align(label1_status, LV_ALIGN_CENTER, 100, 300);
        lv_label_set_text(label1_status, "motor 1 status :");
        label1_speed = lv_label_create(lv_scr_act());
        lv_obj_align(label1_speed, LV_ALIGN_CENTER, 100, 450);
        lv_label_set_text(label1_speed, "motor 1 speed :");
        label1_jitter = lv_label_create(lv_scr_act());
        lv_obj_align(label1_jitter, LV_ALIGN_CENTER, 100, 600);

}

static void display_motor_information(void) {
    int i = -1; 
    i = MADHT1505BA1_check_motor(&slave0);
    // 2000 is Deviation value
    if(i == -1) {
        lv_label_set_text(label0_status, "motor 0 status : Not available");
    }else if(i == 0 || i == 2000 || i == -2000) {
        lv_label_set_text(label0_status, "motor 0 status : stop");
        lv_label_set_text(label0_speed, "motor 0 speed : 0");
    }else {
        char string[100] = {0};
        sprintf(string, "motor 0 speed : %d",i);
        lv_label_set_text(label0_status, "motor 0 status : start");
        lv_label_set_text(label0_speed, string);
    }

    i = MADHT1505BA1_check_motor(&slave1);
    // 2000 is Deviation value
    if(i == -1) {
        lv_label_set_text(label1_status, "motor 1 status : Not available");
    }else if(i == 0 || i == 2000 || i == -2000) {
        lv_label_set_text(label1_status, "motor 1 status : stop");
        lv_label_set_text(label1_speed, "motor 1 speed : 0");
    }else {
        char string1[100] = {0};
        sprintf(string1, "motor 1 speed : %d",i);
        lv_label_set_text(label1_status, "motor 1 status : start");
        lv_label_set_text(label1_speed, string1);
    }
    // uint32_t min = MADHT1505BA1_time_statistics_latency_min_ns();
    // uint32_t max = MADHT1505BA1_time_statistics_latency_max_ns();
    uint32_t min = MADHT1505BA1_time_statistics_period_min_ns();
    uint32_t max = MADHT1505BA1_time_statistics_period_max_ns();
    char string2[100] = {0};
    sprintf(string2, "time jitter: max: %10u  min: %10u",max, min);
    lv_label_set_text(label0_jitter, string2);
    lv_label_set_text(label1_jitter, string2);
}

int main(int argc, char **argv)
{
    int maxpri;
    struct sched_param param;
    int ret = 0;
    if(thread_bind_cpu(1) == -1) {
        printf("bind cpu core fail\n");
    }

    // The scheduling priority is the highest
    maxpri = sched_get_priority_max(SCHED_FIFO);
    if(maxpri == -1) { 
        printf("sched_get_priority_max() failed");
    }

    param.sched_priority = maxpri;
    if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) { 
        perror("sched_setscheduler() failed");
    }

    signal(SIGINT, sigterm_handler);
    lvgl_init();

    while(motor_init() == -1) {
        printf("motor init is err \n");
        sleep(1);
    }

    ui_init();
    
    while (!quit)
    {
        display_motor_information();
        lv_task_handler();
        usleep(10000);
    }

    return 0;
}
