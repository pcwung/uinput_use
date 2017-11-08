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

#define LOG_TAG "UINPUT"
#include <utils/Log.h> //all Android LOG macros are defined here.


#define DEBUG 1
#ifdef DEBUG
#define LOG(lvl, f,...) do{ \
    LOG_PRI(ANDROID_LOG_ERROR,LOG_TAG,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
					  }while(0)
#else
#define LOG(lvl,f,...)	do{}while(0)
#endif

#define SHIFT   (1 << 16)

/*
 * Convert from character to KEY_CODE
 */
static int keyboard_map(const char key)
{
    int event = -1;
    switch (key) {
        case '1': event = KEY_1; break;
        case '2': event = KEY_2; break;
        case '3': event = KEY_3; break;
        case '4': event = KEY_4; break;
        case '5': event = KEY_5; break;
        case '6': event = KEY_6; break;
        case '7': event = KEY_7; break;
        case '8': event = KEY_8; break;
        case '9': event = KEY_9; break;
        case '0': event = KEY_0; break;
        
        case 'a': event = KEY_A; break;
        case 'b': event = KEY_B; break;
        case 'c': event = KEY_C; break;
        case 'd': event = KEY_D; break;
        case 'e': event = KEY_E; break;
        case 'f': event = KEY_F; break;
        case 'g': event = KEY_G; break;
        case 'h': event = KEY_H; break;
        case 'i': event = KEY_I; break;
        case 'j': event = KEY_J; break;
        case 'k': event = KEY_K; break;
        case 'l': event = KEY_L; break;
        case 'm': event = KEY_M; break;
        case 'n': event = KEY_N; break;
        case 'o': event = KEY_O; break;
        case 'p': event = KEY_P; break;
        case 'q': event = KEY_Q; break;
        case 'r': event = KEY_R; break;
        case 's': event = KEY_S; break;
        case 't': event = KEY_T; break;
        case 'u': event = KEY_U; break;
        case 'v': event = KEY_V; break;
        case 'w': event = KEY_W; break;
        case 'x': event = KEY_X; break;
        case 'y': event = KEY_Y; break;
        case 'z': event = KEY_Z; break;
        
        case 'A': event = KEY_A; event |= SHIFT; break;
        case 'B': event = KEY_B; event |= SHIFT; break;
        case 'C': event = KEY_C; event |= SHIFT; break;
        case 'D': event = KEY_D; event |= SHIFT; break;
        case 'E': event = KEY_E; event |= SHIFT; break;
        case 'F': event = KEY_F; event |= SHIFT; break;
        case 'G': event = KEY_G; event |= SHIFT; break;
        case 'H': event = KEY_H; event |= SHIFT; break;
        case 'I': event = KEY_I; event |= SHIFT; break;
        case 'J': event = KEY_J; event |= SHIFT; break;
        case 'K': event = KEY_K; event |= SHIFT; break;
        case 'L': event = KEY_L; event |= SHIFT; break;
        case 'M': event = KEY_M; event |= SHIFT; break;
        case 'N': event = KEY_N; event |= SHIFT; break;
        case 'O': event = KEY_O; event |= SHIFT; break;
        case 'P': event = KEY_P; event |= SHIFT; break;
        case 'Q': event = KEY_Q; event |= SHIFT; break;
        case 'R': event = KEY_R; event |= SHIFT; break;
        case 'S': event = KEY_S; event |= SHIFT; break;
        case 'T': event = KEY_T; event |= SHIFT; break;
        case 'U': event = KEY_U; event |= SHIFT; break;
        case 'V': event = KEY_V; event |= SHIFT; break;
        case 'W': event = KEY_W; event |= SHIFT; break;
        case 'X': event = KEY_X; event |= SHIFT; break;
        case 'Y': event = KEY_Y; event |= SHIFT; break;
        case 'Z': event = KEY_Z; event |= SHIFT; break;
        
        case '!': event = KEY_1; event |= SHIFT; break;
        case '@': event = KEY_2; event |= SHIFT; break;
        case '#': event = KEY_3; event |= SHIFT; break;
        case '$': event = KEY_4; event |= SHIFT; break;
        case '%': event = KEY_5; event |= SHIFT;  break;
        case '^': event = KEY_6; event |= SHIFT; break;
        case '&': event = KEY_7; event |= SHIFT; break;
        case '*': event = KEY_8; event |= SHIFT; break;
        case '(': event = KEY_9; event |= SHIFT; break;
        case ')': event = KEY_0; event |= SHIFT; break;

        case '-': event = KEY_MINUS; break;
        case '_': event = KEY_MINUS; event |= SHIFT; break;
        case '=': event = KEY_EQUAL; break;
        case '+': event = KEY_EQUAL; event |= SHIFT; break;
        case ';': event = KEY_SEMICOLON; break;
        case ':': event = KEY_SEMICOLON; event |= SHIFT; break;
        case ',': event = KEY_COMMA; break;
        case '<': event = KEY_COMMA; event |= SHIFT; break;
        case '.': event = KEY_DOT; break;
        case '>': event = KEY_DOT; event |= SHIFT; break;
        case '/': event = KEY_SLASH; break;
        case '?': event = KEY_SLASH; event |= SHIFT; break;
        case '[': event = KEY_LEFTBRACE; break;
        case '{': event = KEY_LEFTBRACE; event |= SHIFT; break;
        case ']': event = KEY_RIGHTBRACE; break;
        case '}': event = KEY_RIGHTBRACE; event |= SHIFT; break;
        case '\\': event = KEY_BACKSLASH; break;
        case '|': event = KEY_BACKSLASH; event |= SHIFT; break;
        case '\'': event = KEY_APOSTROPHE; break;
        case '"': event = KEY_APOSTROPHE; event |= SHIFT; break;
        case '`': event = KEY_GRAVE; break;
        case '~': event = KEY_BACKSLASH; event |= SHIFT; break;

        default:
            event = -1;
            break;
    }
    return event;
}

int keyboard_init(void)
{
    int                    fd;
    struct uinput_user_dev uidev;
    //struct input_event     ev;
    int                    i;
    
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd <= 0) {
        LOG(0, "error: open");
        return -1;
    } else {
        LOG(0, "open /dev/uinput fd = %d\n", fd);
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "HID Fake Device");
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
        LOG(0, "error: write");
        return -1;
    }

    if(ioctl(fd, UI_DEV_CREATE) < 0) {
        LOG(0, "error: ioctl");
        return -1;
    }

    return fd;
}

int keyboard_release(int fd)
{
    if (fd <= 0)
        return -1;
    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        LOG(0, "error: ioctl");

    close(fd);
    return 0;
}

int keyboard_write(int fd, const char key)
{
    struct input_event event;
    const int kval = keyboard_map(key);
    const unsigned short key_code = (unsigned short) kval;

    if (fd <= 0) {
        LOG(0, "open fd <= 0 error!\n");
        return -1;
    }
    
    if (kval <= 0) {
        LOG(0, "keyboard_write error!\n");
        return -1;
    }

    // shift down
    if ((kval & SHIFT) == SHIFT) {
        memset(&event, 0, sizeof(event));
        gettimeofday(&event.time, 0);
        event.type = EV_KEY;
        event.value = 1;
        event.code = KEY_LEFTSHIFT;
        if (write(fd, &event, sizeof(event)) < 0)
            return -1;
    }

    // key down
    LOG(0, "key down \n");
    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, 0);
    event.type = EV_KEY;
    event.value = 1;
    event.code = key_code;
    if (write(fd, &event, sizeof(event)) < 0)
        return -1;

   usleep(100);
    // key up
    LOG(0, "key up \n");
    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, 0);
    event.type = EV_KEY;
    event.value = 0;
    event.code = key_code;
    if (write(fd, &event, sizeof(event)) < 0)
        return -1;

    // shift down
    if ((kval & SHIFT) == SHIFT) {
        memset(&event, 0, sizeof(event));
        gettimeofday(&event.time, 0);
        event.type = EV_KEY;
        event.value = 0;
        event.code = KEY_LEFTSHIFT;
        if (write(fd, &event, sizeof(event)) < 0)
            return -1;
    }

    // sync
    LOG(0, "key sync \n");
    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    if (write(fd, &event, sizeof(event)) < 0)
        return -1;

    return 0;
}

int main(int argc, char **argv)
{
	int fd = -1;
	int i = -1;
	const char *buf = "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	
	fd = keyboard_init();
	if (fd <= 0) {
		LOG(0, "error: keyboard_init()");
		return -1;
	}
/*
	for (i = 0; i < strlen(buf); i++) {
        keyboard_write(fd, buf[i]);
        usleep(10000);
    }
   */ 
    usleep(10000000);
    keyboard_write(fd,0);
    usleep(10000000);
    keyboard_release(fd);
    
    return 0;
}