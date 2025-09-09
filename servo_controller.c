#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define KEY_W 17
#define KEY_S 31

int main(){
	struct input_event ev;
	
	char *device_1 = "/dev/input/event0";
	char *device_2 = "/dev/key_test";

	int fd_1 = open(device_1, O_RDONLY);
	int fd_2;
	int fd_2_read;
	char buf[32] = {0};
	ssize_t bytes_read;

//	fcntl(fd_1, F_SETFL, fcntl(fd_1, F_GETFL) | O_NONBLOCK);

	if(fd_1 < 0){
		perror("Failed to open device");
		return 1;
	}

	while (1) {
		ssize_t n = read(fd_1, &ev, sizeof(ev));
		if (n < 0){
			perror("Failed to read event\n");
			break;

		} else if (n != sizeof(ev)){
			fprintf(stderr, "failed to read: expected %zu, got %zd\n", sizeof(ev), n);
			continue;
		}
		
		if (ev.type == EV_KEY && ev.code == KEY_W) {
						
			fd_2 = open(device_2, O_WRONLY);
			write(fd_2, "W", 1);
			close(fd_2);

			//buf[32] = {0};
			fd_2_read = open(device_2, O_RDONLY);
			bytes_read = read(fd_2_read, buf, sizeof(buf) - 1);
			close(fd_2_read);

			buf[bytes_read] = '\0';
			printf("%s", buf);

		} else if (ev.type == EV_KEY && ev.code == KEY_S) {
			fd_2 = open(device_2, O_WRONLY);
			write(fd_2, "S", 1);
			close(fd_2);

			//buf[32] = {0};
			fd_2_read = open(device_2, O_RDONLY);
			bytes_read = read(fd_2_read, buf, sizeof(buf) - 1);
			close(fd_2_read);

			buf[bytes_read] = '\0';
			printf("%s", buf);

		}

//		if (n == sizeof(ev)){
//			printf("Type: 0x%02x, Code: 0x%02x, Value: %d\n", ev.type, ev.code, ev.value);
//		} else {
//			perror("Failed to read event\n");
//			break;
//		}
	}

	close(fd_1);
//	close(fd_2);


	return EXIT_SUCCESS;
}

