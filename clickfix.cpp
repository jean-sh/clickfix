#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <linux/input.h>
#include <linux/uinput.h>
#include <experimental/filesystem>
#include <chrono>

const std::string ERROR_IOCTL = "error: ioctl";
const std::string ERROR_OPEN = "error: open";
const std::string ERROR_WRITE = "error: write";
const int TRIGGER_THRESHOLD = 90000;
	
void die(const std::string& message)
{
	std::cerr << message << "\n";
	exit(EXIT_FAILURE);
}

int create_uinput()
{
	namespace fs = std::experimental::filesystem;
	using std::string;
	
	/* Find where uinput is */
	const fs::path dev_uinput = "/dev/uinput";
	const fs::path dev_input_uinput = "/dev/input/uinput";
	string uinput_path;
	
	if (fs::exists(dev_uinput)) {
		uinput_path = dev_uinput.string();
	} else if (fs::exists(dev_input_uinput)) {
		uinput_path = dev_input_uinput.string();
	} else {
		die("Could not find uinput device.");
	}
	std::cout << "Found uinput device: " << uinput_path << ".\n";
	
	/* Open uinput */
    const int fd = open(uinput_path.c_str(), O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
        die(ERROR_OPEN);
	}

	/* Set all bits we need for a mouse */
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        die(ERROR_IOCTL);
	}
	
    if (ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0) {
        die(ERROR_IOCTL);
	}
	
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_SIDE) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_EXTRA) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_FORWARD) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_BACK) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_TASK) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_KEYBIT, BTN_EXTRA) < 0) {
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
	
	if (ioctl(fd, UI_SET_RELBIT, REL_WHEEL) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_RELBIT, REL_HWHEEL) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_RELBIT, REL_MISC) < 0) {
		die(ERROR_IOCTL);
	}
	
	if (ioctl(fd, UI_SET_RELBIT, REL_MAX) < 0) {
		die(ERROR_IOCTL);
	}

	/* Create the device */
	struct uinput_user_dev uidev;
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

void inject_event(const int fd, const input_event& event)
{
	/* Writing the event to uinput */
	input_event ev;
	memset(&ev, 0, sizeof(input_event));
	ev.type = event.type;
	ev.code = event.code;
	ev.value = event.value;

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

uint64_t get_timeval_usec(const timeval& t)
{
	return t.tv_sec * 1000000 + t.tv_usec;
}

void print_timestamp(const int value)
{
	using namespace std::chrono;
	time_t cur = system_clock::to_time_t(system_clock::now());
	std::cout << "value: " << value << " | " << std::ctime(&cur);
}

int main()
{
	namespace fs = std::experimental::filesystem;
	using std::string;

	/*
	 * Find the path to the event mouse
	 */
	string device_path;
	const string dev_input = "/dev/input";
	fs::directory_iterator it(dev_input + "/by-path");
	for (auto f : it) {
		if (f.path().string().find("event-mouse") != string::npos) {
			string tmp;
			tmp = fs::read_symlink(f.path()).string();
			device_path = dev_input + tmp.substr(2);
			std::cout << "Found event mouse: " << device_path << ".\n";
			break;
		}
	}

	/*
	 * Open the specified device.
	 */
	int fd;
	do {
		fd = open(device_path.c_str(), O_RDONLY);
	} while (fd == -1 && errno == EINTR);
	if (fd == -1) {
		std::cerr << device_path << ": " << strerror(errno) << ".\n";
		return 1;
	}

	/*
	 * Use EVIOCGNAME to get the device name.
	 */
	char device_name[64];
	memset(device_name, 0, sizeof device_name);
	ioctl(fd, EVIOCGNAME(sizeof device_name - 1), device_name); 

	if (strlen(device_name) > 0) {
		std::cerr << "Mouse name: " << device_name << ".\n";
	}

	errno = 0;
	if (ioctl(fd, EVIOCGRAB, 1) == 0) {
		std::cerr << "Device grabbed successfully.\n";
	} else {
		std::cerr << "Failed to grab device (" << strerror(errno) << ").\n";
	}

	// Create the virtual device to inject events into.
	const int fd_uinput = create_uinput();

	/*
	 * Intercept event loop
	 */
	input_event ev;
	ssize_t bytes_read;
	uint64_t prev_event[2] = {0, 0};
	uint64_t delta;
	while (1) {
		// TODO: better flow
		/* Read the event. */
		bytes_read = read(fd, &ev, sizeof ev);
		if (bytes_read == -1) {
			/* It is not an error if the read was interrupted. */
			if (errno == EINTR) {
				continue;
			}
			/* Print an error message, and break out of loop. */
			std::cerr << device_path << ": " << strerror(errno) << ".\n";
			break;
		} else {
			if (bytes_read == 0) {
				/* End of input; device detached? */
				std::cerr << device_path << ": No more events.\n";
				break;
			} else {
				if (bytes_read != sizeof ev) {
					/* This should never occur; input driver or kernel bug. */
					std::cerr << device_path << ": Invalid event (length "
						<< (int)bytes_read << ", expected " << (int)sizeof ev << ")\n";
					/* We just ignore those, and wait for next event. */
					continue;
				}
			}
		}
		
		/* Always inject non left button events */
		if (ev.code != BTN_LEFT) {
			inject_event(fd_uinput, ev);
			continue;
		}
		
		/* In case of left button event, ignore if delta is below threshold. */
		delta = get_timeval_usec(ev.time) - prev_event[ev.value];
		if (delta < TRIGGER_THRESHOLD) {
			print_timestamp(ev.value);
		} else {
			inject_event(fd_uinput, ev);
		}
		prev_event[ev.value] = get_timeval_usec(ev.time);
	}

	destroy_uinput(fd_uinput);
	close(fd);

	return 0;
}
