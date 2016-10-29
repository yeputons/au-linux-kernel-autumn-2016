#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <vsd_ioctl.h>

#include <exception>
#include <system_error>
#include <iostream>
#include <string>

class syscall_error : public std::system_error {
public:
    syscall_error(const std::string &reason)
        : system_error(errno, std::system_category(), reason) {}
};

class illegal_argv_error : public std::runtime_error {
public:
    illegal_argv_error(const std::string &reason)
        : runtime_error("Invalid arguments: " + reason) {}
};

long parse_arg_long(char *arg) {
    try {
        std::size_t pos = 0;
        long res = std::stol(arg, &pos);
	if (arg[0] == '\0' || arg[pos] != '\0') {
	    throw std::invalid_argument("");
	}
	return res;
    } catch (std::invalid_argument &e) {
        throw illegal_argv_error("\"" + std::string(arg) + "\" is not a valid number");
    } catch (std::out_of_range &e) {
        throw illegal_argv_error("\"" + std::string(arg) + "\" is out of range");
    }
}

class Vsd {
public:
    Vsd(const char *devname) {
        fd_ = open(devname, 0);
	if (fd_ < 0)
	    throw syscall_error("Unable to open vsd");
    }
    ~Vsd() { close(fd_); }

    long size_get() {
        vsd_ioctl_get_size_arg arg;
        if (ioctl(fd_, VSD_IOCTL_GET_SIZE, &arg))
	    throw syscall_error("Unable to VSD_IOCTL_GET_SIZE");
	return arg.size;
    }
    
    void size_set(long size) {
        vsd_ioctl_set_size_arg arg;
	arg.size = size;
        if (ioctl(fd_, VSD_IOCTL_SET_SIZE, &arg))
	    throw syscall_error("Unable to VSD_IOCTL_SET_SIZE");
    }

private:
    int fd_;
};

int size_get(int argc, char*[]) {
    if (argc != 0)
        throw illegal_argv_error("too much arguments");
    Vsd vsd("/dev/vsd");
    std::cout << vsd.size_get() << std::endl;
    return EXIT_SUCCESS;
}

int size_set(int argc, char* argv[]) {
    if (argc < 1)
        throw illegal_argv_error("not enough arguments");
    if (argc > 1)
        throw illegal_argv_error("too much arguments");
    
    Vsd vsd("/dev/vsd");
    vsd.size_set(parse_arg_long(argv[0]));
    return EXIT_SUCCESS;
}

void help() {
    std::cout <<
        "Usage: ./vsd_userspace <command> <args...>\n"
	"Available commands:\n"
	"    size_get - no arguments, prints current vsd size\n"
	"    size_set <new_size> - sets vsd size to <new_size>\n"
        ;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Not enough arguments" << std::endl;
        help();
	return EXIT_FAILURE;
    }

    try {
        if (!std::strcmp(argv[1], "size_get")) {
            return size_get(argc - 2, argv + 2);
        } else if (!std::strcmp(argv[1], "size_set")) {
            return size_set(argc - 2, argv + 2);
        } else {
            std::cerr << "Invalid command: " << argv[1] << std::endl;
            help();
            return EXIT_FAILURE;
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
    }
}
