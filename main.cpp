#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <linux/input.h>
#include <linux/uinput.h>

// TODO: Profile << vs printf

const std::string ERROR_IOCTL = "error: ioctl";
const std::string ERROR_OPEN = "error: open";
const std::string ERROR_WRITE = "error: write";
	
void die(const std::string message)
{
	std::cerr << message << "\n";
	exit(EXIT_FAILURE);
}

int create_uinput()
{
	int fd;
    struct uinput_user_dev uidev;
	
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if (fd < 0) {
        die(ERROR_OPEN);
	}

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        die(ERROR_IOCTL);
	}
	
    if (ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0) {
        die(ERROR_IOCTL);
	}
	
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
		die(ERROR_IOCTL);
	}
	
    if (ioctl(fd, UI_SET_EVBIT, EV_REL) < 0) {
        die(ERROR_IOCTL);
	}
	
    if (ioctl(fd, UI_SET_RELBIT, REL_X) < 0) {
        die(ERROR_IOCTL);
	}
	
    if (ioctl(fd, UI_SET_RELBIT, REL_Y) < 0) {
        die(ERROR_IOCTL);
	}

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "click-fixer");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if (write(fd, &uidev, sizeof(uidev)) < 0) {
        die(ERROR_OPEN);
	}

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        die(ERROR_IOCTL);
	}
	
	return fd;
}

void destroy_uinput(const int fd)
{
	if (ioctl(fd, UI_DEV_DESTROY) < 0) {
        die(ERROR_IOCTL);
	}
	
	close(fd);
}

void inject_event(const int fd, input_event& event)
{
	/* Writing the event to uinput */
	input_event ev;
	memset(&ev, 0, sizeof(input_event));
	ev.type = event.type;
	ev.code = event.code;
	ev.value = event.value;
	
	std::cout << "ev.type: " << ev.type << " ev.code: " << ev.code << " ev.value: " << ev.value << "\n";
	
	if (write(fd, &ev, sizeof(input_event)) < 0) {
		std::cerr << ERROR_OPEN << "\n";
	}
	
	/* sync */
	memset(&ev, 0, sizeof(input_event));
		ev.type = EV_SYN;
		ev.code = 0;
		ev.value = 0;
		if (write(fd, &ev, sizeof(input_event)) < 0) {
			std::cerr << ERROR_OPEN << "\n";
		}
}

bool check_double_click() {
	
}

int main(int argc, char* argv[])
{
	int fd_uinput = create_uinput();
	usleep(500000);
	char device_name[64];
	input_event ev;
	char* truedev;
	ssize_t n;
	int fd;

	if (argc != 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		std::cerr << "\n";
		std::cerr << "Usage: " << argv[0] << " [..-h | --help ]\n";
		std::cerr << "       " << argv[0] << " /dev/input/event0\n";
		std::cerr << "\n";
		std::cerr << "This example program describes mouse events.\n";
		std::cerr << "\n";
		return 0;
	}

	/* Open the specified device. */
	do {
		fd = open(argv[1], O_RDONLY);
	} while (fd == -1 && errno == EINTR);
	if (fd == -1) {
		std::cerr << argv[1] << ": " << strerror(errno) << ".\n";
		return 1;
	}

	/* Use EVIOCGNAME to get the device name. (First we clear it, though.) */
	memset(device_name, 0, sizeof device_name);
	ioctl(fd, EVIOCGNAME(sizeof device_name - 1), device_name); 

	/* Obtain real, absolute path to the device. */
	truedev = canonicalize_file_name(argv[1]);
	if (truedev != NULL)
		std::cerr << "Reading from " << truedev << " (" << argv[1] << ")\n";
	else
		std::cerr << "Reading from " << argv[1] << "\n";

	if (strlen(device_name) > 0)
		std::cerr << "EVIOCGNAME ioctl reports device is named '" << device_name << "'.\n";

	errno = 0;
	if (ioctl(fd, EVIOCGRAB, 1) == 0)
		std::cerr << "Device grabbed (using EVIOCGRAB ioctl) successfully.\n";
	else
		std::cerr << "Failed to grab device (" << strerror(errno) << ").\n";
	
	/* Read input events (forever). */
	while (1) {

		// TODO: Make these ifs simpler.
		n = read(fd, &ev, sizeof ev);
		if (n == -1) {
			/* It is not an error if the read was interrupted. */
			if (errno == EINTR) {
				continue;
			}
			/* Print an error message, and break out of loop. */
			std::cerr << argv[1] << ": " << strerror(errno) << ".\n";
			break;
		} else {
			if (n == 0) {
				/* End of input; device detached? */
				std::cerr << argv[1] << ": No more events.\n";
				break;
			} else {
				if (n != sizeof ev) {
					/* This should never occur; input driver or kernel bug. */
					std::cerr << argv[1] << ": Invalid event (length " << (int)n << ", expected " << (int)sizeof ev<< ")\n";
					/* We just ignore those, and wait for next event. */
					continue;
				}
			}
		}
		
		inject_event(fd_uinput, ev);
	}

	close(fd);
	
	return 0;
}
