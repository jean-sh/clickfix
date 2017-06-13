#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <linux/input.h>
#include <linux/uinput.h>

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

const char MICE_FILE[] = "/dev/input/mice";
const char EVENT3[] = "/dev/input/event3";

void detect_click()
{
    int fd_mice;
    if ((fd_mice = open(MICE_FILE, O_RDONLY)) == -1) {
        std::cout << "Device open ERROR\n";
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Device open OK\n";
    }
    
    struct input_event ev;
    unsigned char button;
    while (read(fd_mice, &ev, sizeof(struct input_event))) {
        unsigned char *ptr = (unsigned char*)&ev;
        button = ptr[0];
        bool left_click = button & 0x1;
        if (left_click) {
			std::cout << "Click !\n";
		}
    }
}

void write_to_uinput() {
	int fd_uinput;
	fd_uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd_uinput == -1) {
		exit(EXIT_FAILURE);
	}
	std::cout << fd_uinput;

	int ret;
	ret = ioctl(fd_uinput, UI_SET_EVBIT, EV_KEY);
	ret = ioctl(fd_uinput, UI_SET_EVBIT, EV_SYN);
	ret = ioctl(fd_uinput, UI_SET_KEYBIT, KEY_D);
	
	struct uinput_user_dev uidev;
	memset(&uidev, 0, sizeof(uidev));
	
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor = 0x1234;
	uidev.id.product = 0xfedc;
	uidev.id.version = 1;
	
	ret = write(fd_uinput, &uidev, sizeof(uidev));
	ret = ioctl(fd_uinput, UI_DEV_CREATE);
	
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));

	ev.type = EV_KEY;
	ev.code = KEY_D;
	ev.value = 1;
	
	ret = write(fd_uinput, &ev, sizeof(ev));

}

void grab_mice()
{
	int fd_mice;
	fd_mice = open(EVENT3, O_RDWR);
	ioctl(fd_mice, EVIOCGRAB, 1);
	int n;
	struct input_event ev;
	while (1) {
		n = read(fd_mice, &ev, sizeof ev);
	}
}

void inject_in_kbd(struct input_event event) {
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	
	int fd = open("/dev/input/event2", O_WRONLY | O_NONBLOCK);
	int ret;
	
	ev.type = EV_KEY;
	ev.code = KEY_D;
	ev.value = 1;
	ret = write(fd, &ev, sizeof(ev));
	
	ev.type = EV_KEY;
	ev.code = KEY_D;
	ev.value = 0;
	ret = write(fd, &ev, sizeof(ev));
	
	close(fd);
}

void uinput_sample() {
	int                    fd;
    struct uinput_user_dev uidev;
    struct input_event     ev;
    int                    dx, dy;
    int                    i;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if(fd < 0)
        die("error: open");

    if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
        die("error: ioctl");

    if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
        die("error: ioctl");

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(fd, &uidev, sizeof(uidev)) < 0)
        die("error: write");

    if(ioctl(fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");

    sleep(2);

    srand(time(NULL));

    while(1) {
        switch(rand() % 4) {
        case 0:
            dx = -10;
            dy = -1;
            break;
        case 1:
            dx = 10;
            dy = 1;
            break;
        case 2:
            dx = -1;
            dy = 10;
            break;
        case 3:
            dx = 1;
            dy = -10;
            break;
        }

        for(i = 0; i < 20; i++) {
            memset(&ev, 0, sizeof(struct input_event));
            ev.type = EV_REL;
            ev.code = REL_X;
            ev.value = dx;
            if(write(fd, &ev, sizeof(struct input_event)) < 0)
                die("error: write");

            memset(&ev, 0, sizeof(struct input_event));
            ev.type = EV_REL;
            ev.code = REL_Y;
            ev.value = dy;
            if(write(fd, &ev, sizeof(struct input_event)) < 0)
                die("error: write");

            memset(&ev, 0, sizeof(struct input_event));
            ev.type = EV_SYN;
            ev.code = 0;
            ev.value = 0;
            if(write(fd, &ev, sizeof(struct input_event)) < 0)
                die("error: write");

            usleep(15000);
        }

        sleep(5);
    }

    sleep(2);

    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        die("error: ioctl");

    close(fd);
}

int open_uinput() {
	int fd;
    struct uinput_user_dev uidev;
	
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if(fd < 0)
        die("error: open");

    if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
        die("error: ioctl");
	if(ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
		die("error:ioctl");
	}
    if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
        die("error: ioctl");

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(fd, &uidev, sizeof(uidev)) < 0)
        die("error: write");

    if(ioctl(fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");
	
	return fd;
}

void close_uinput(int* fd) {
	if(ioctl(*fd, UI_DEV_DESTROY) < 0) {
        die("error: ioctl");
	}
	close(*fd);
}

void inject_mouse_events(int* fd, struct input_event event_to_inject) {
	
	/* Writing the event to uinput */
	struct input_event event;
	memset(&event, 0, sizeof(struct input_event));
	event.type = event_to_inject.type;
	event.code = event_to_inject.code;
	event.value = event_to_inject.value;
	std::cout << "event.type: " << event.type << " event.code: " << event.code << " event.value: " << event.value << "\n";
	if (write(*fd, &event, sizeof(struct input_event)) < 0) {
		//die("error: write");
	}
	
	/* sync */
	memset(&event, 0, sizeof(struct input_event));
		event.type = EV_SYN;
		event.code = 0;
		event.value = 0;
		if(write(*fd, &event, sizeof(struct input_event)) < 0) {
			//die("error: write");
		}
}

int main(int argc, char* argv[])
{
	int v = 0;
	int fd_uinput = open_uinput();
	sleep(1);
	switch (v) {
		case 0:
			char device_name[64];
			struct input_event ev;
			char *truedev;
			ssize_t n;
			int fd;
		
			if (argc != 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
				fprintf(stderr, "\n");
				fprintf(stderr, "Usage: %s [..-h | --help ]\n", argv[0]);
				fprintf(stderr, "       %s /dev/input/event0\n", argv[0]);
				fprintf(stderr, "\n");
				fprintf(stderr, "This example program describes mouse events.\n");
				fprintf(stderr, "\n");
				return 0;
			}
		
			/* Open the specified device. */
			do {
				fd = open(argv[1], O_RDONLY);
			} while (fd == -1 && errno == EINTR);
			if (fd == -1) {
				fprintf(stderr, "%s: %s.\n", argv[1], strerror(errno));
				return 1;
			}
		
			/* Use EVIOCGNAME to get the device name. (First we clear it, though.) */
			memset(device_name, 0, sizeof device_name);
			ioctl(fd, EVIOCGNAME(sizeof device_name - 1), device_name); 
		
			/* Obtain real, absolute path to the device. */
			truedev = canonicalize_file_name(argv[1]);
			if (truedev != NULL)
				fprintf(stderr, "Reading from %s (%s)\n", truedev, argv[1]);
			else
				fprintf(stderr, "Reading from %s\n", argv[1]);
		
			if (strlen(device_name) > 0)
				fprintf(stderr, "EVIOCGNAME ioctl reports device is named '%s'.\n", device_name);
		
			errno = 0;
			if (ioctl(fd, EVIOCGRAB, 1/*0*/) == 0)
				fprintf(stderr, "Device grabbed (using EVIOCGRAB ioctl) successfully.\n");
			else
				fprintf(stderr, "Failed to grab device (%s).\n", strerror(errno));
		
			fflush(stderr);
		
			/* Open the file to inject events into ***************************************************************/
			
			/* Read input events (forever). */
			while (1) {
		
				n = read(fd, &ev, sizeof ev);
				if (n == -1) {
					/* It is not an error if the read was interrupted. */
					if (errno == EINTR)
						continue;
					/* Print an error message, and break out of loop. */
					fprintf(stderr, "%s: %s.\n", argv[1], strerror(errno));
					break;
				} else
				if (n == 0) {
					/* End of input; device detached? */
					fprintf(stderr, "%s: No more events.\n", argv[1]);
					break;
				} else
				if (n != sizeof ev) {
					/* This should never occur; input driver or kernel bug. */
					fprintf(stderr, "%s: Invalid event (length %d, expected %d)\n",
							argv[1], (int)n, (int)sizeof ev);
					/* We just ignore those, and wait for next event. */
					continue;
				}
				inject_mouse_events(&fd_uinput, ev);
			}
		
			close(fd);
			break;
		case 1:
			uinput_sample();
			break;
		case 2:
			int dx = 100;
			struct input_event ev2;
			ev2.type = EV_REL;
			ev2.code = REL_X;
			ev2.value = dx;
			inject_mouse_events(&fd_uinput, ev2);
			break;
	}
	
	return 0;
}
