#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
#include <hardware_legacy/power.h>
#include <utils/Log.h>

#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "jni.h"

#define LOG_TAG "UINPUT"
#include <utils/Log.h> //all Android LOG macros are defined here.


#define SHIFT   (1 << 16)
int                    fd;


int keyboard_init(void)
{
    struct uinput_user_dev uidev;
    //struct input_event     ev;
    int                    i;
    
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd <= 0) {
        ALOGI("error: open");
        return -1;
    } else {
        ALOGI("open /dev/uinput fd = %d\n", fd);
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "wade_remote");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    // config uinput device Keyboard
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for (i = 0; i < 128; i++) {
        ioctl(fd, UI_SET_KEYBIT, i);
        usleep(10000);
    }
    
    if(write(fd, &uidev, sizeof(uidev)) < 0) {
        ALOGI("error: write");
        return -1;
    }

    if(ioctl(fd, UI_DEV_CREATE) < 0) {
        ALOGI("error: ioctl");
        return -1;
    }

    return fd;
}

int keyboard_release(int fd)
{
    if (fd <= 0)
        return -1;
    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        ALOGI("error: ioctl");

    close(fd);
    return 0;
}

int keyboard_write(int fd, const char key, bool up_down)
{
    struct input_event event;
    const unsigned short key_code = (unsigned short) key;

    if (fd <= 0) {
        ALOGI("open fd <= 0 error!\n");
        return -1;
    }
    
    // key down
   if (up_down == 0){
    		ALOGI("key down \n");
	    memset(&event, 0, sizeof(event));
	    gettimeofday(&event.time, 0);
	    event.type = EV_KEY;
	    event.value = 1;
	    event.code = key_code;
	    if (write(fd, &event, sizeof(event)) < 0)
	        return -1;
	   
 	}
    // key up
     if (up_down == 1){
	    ALOGI("key up \n");
	    memset(&event, 0, sizeof(event));
	    gettimeofday(&event.time, 0);
	    event.type = EV_KEY;
	    event.value = 0;
	    event.code = key_code;
	    if (write(fd, &event, sizeof(event)) < 0)
	        return -1;

	     
	         // sync
	         //edit by Wade when key up send syn
	    ALOGI("key sync \n");
	    event.type = EV_SYN;
	    event.value = 0;
	    event.code = SYN_REPORT;
	    if (write(fd, &event, sizeof(event)) < 0)
        return -1;


	}
	

    return 0;
}



static int native_ukeyreport(JNIEnv *env, jobject thiz,int key_value, bool up_down)
{
	if (fd < 1) {
		ALOGI("ukeyreport  failed!------,type=%d,open=%d",key_value, up_down);
		return -EFAULT;
	}
	int ret = keyboard_write(fd, key_value, up_down);
	return ret;
}

//=======================================================================

static const char *classPathName = "com/mgexample/bluetooth/remotecontrol/uinput";

static JNINativeMethod methods[] = {
  {"ukeyreport", "(IZ)I", (void*)native_ukeyreport },
  //{"get_status", "(I)I", (void*)native_get_status },
};


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */
 
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jint result = -1;
    JNIEnv* env = NULL;
    jclass clazz;
    ALOGI("JNI_OnLoad");

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }
    
	fd = keyboard_init();
	if (fd <= 0) {
		ALOGE("ERROR: error: keyboard_init()");
		return -1;
	}
    result = JNI_VERSION_1_4;
    clazz = env->FindClass(classPathName);
    ALOGI("JNI_registerNatives");
    env->RegisterNatives(clazz,methods, sizeof(methods) / sizeof(methods[0]));
    ALOGI("JNI_registerNatives end");
bail:
    return result;
}

//onUnLoad方法，在JNI组件被释放时调用  
void JNI_OnUnload(JavaVM* vm, void* reserved){  
     ALOGE("call JNI_OnUnload ~~!!");
     if (fd > 0){
     	keyboard_release(fd);
    }  
}  
