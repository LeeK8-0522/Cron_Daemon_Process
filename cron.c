#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

void handle_signal(int signal) {
	syslog(LOG_INFO, "signal detected.");
	closelog();
	exit(signal);
}//for handling signal

void handle_child(int signal) {
	waitpid(-1, NULL, WNOHANG);
}//for reaping child processes

int main(void)
{
	unsigned int pid;//for storing process id
	time_t t;//time
	struct tm *tm;//time struct	
	int fd;//file descripter
	char *argv[3];//arguments
	char buf[512];//string buffer
	int fd0, fd1, fd2;//three file descripters

	fd = open("./crontab", O_RDWR);//open crontab file in read and write mode in current directory 
	pid = fork();//make child process
	
	if(pid == -1) return -1;//if it failes, end program
	if(pid != 0)//kill parent process
		exit(0);
	if(setsid() < 0)//if it failes, kill child process
		exit(0);
	if(chdir("/") < 0)//change dir to root dir. if it failes kill child process
		exit(0);

	umask(0);

	close(0);
	close(1);
	close(2);

	fd0 = open("/dev/null", O_RDWR);
	fd1 = open("/dev/null", O_RDWR);
	fd2 = open("/dev/null", O_RDWR);

	t = time(NULL);
	tm = localtime(&t);

	signal(SIGTERM, handle_signal);//if there is a signal, handle it appropriately
	signal(SIGCHLD, handle_child);

	close(fd);

	while (1)
	{	
		openlog("Daemon Process", LOG_PID | LOG_CONS, LOG_USER);
		FILE* file = fopen("/home/leek8/WEEK7/crontab", "r");
		if(file == NULL) {
			syslog(LOG_ERR, "failed to open file: %s, error: %s", "./crontab", strerror(errno));
		}
		syslog(LOG_INFO, "loop started!");
		while(fgets(buf, sizeof(buf), file) != NULL) {
			t = time(NULL);
			tm = localtime(&t);
			sleep(60 - tm->tm_sec % 60);
			syslog(LOG_INFO, "I am in the loop!");
			buf[strcspn(buf, "\n")] = '\0';
			syslog(LOG_INFO, "I changed string buffer~");

			int i = 0;
			char* saveptr;
			argv[0] = strtok_r(buf, " ", &saveptr);
			argv[1] = strtok_r(NULL, " ", &saveptr);
			argv[2] = strtok_r(NULL, " ", &saveptr);
			syslog(LOG_DEBUG, "%s %s %s", argv[0], argv[1], argv[2]);	
			if(strcmp(argv[0], "*") == 0&& strcmp(argv[1], "*") == 0) {//"* *"
				syslog(LOG_DEBUG, "condition '* *' matched. (%s %s)(%d %d)", argv[0], argv[1], tm->tm_min, tm->tm_hour);
				pid = fork();
				if(pid == 0) {
					execl("/bin/sh", "/bin/sh", "-c", argv[2], (char *)NULL);

					syslog(LOG_ERR, "exec failed (%s)", strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			else if(strcmp(argv[0], "*") == 0&& strcmp(argv[1], "*") != 0) {//"* (num)"
				syslog(LOG_DEBUG, "condition '* (num)' matched. (%s %s)(%d %d)", argv[0], argv[1], tm->tm_min, tm->tm_hour);
				if(tm->tm_hour == atoi(argv[1])) {
					pid = fork();
					if(pid == 0) {
						execl("/bin/sh", "/bin/sh", "-c", argv[2], (char *)NULL);

						syslog(LOG_ERR, "exec failed (%s)", strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
			}
			else if(strcmp(argv[0], "*") != 0 && strcmp(argv[1], "*") == 0) {//"(num) *"
				syslog(LOG_DEBUG, "condition '(num) *' matched. (%s %s)(%d %d)", argv[0], argv[1], tm->tm_min, tm->tm_hour);
				if(tm->tm_min == atoi(argv[0])) {
					pid = fork();
					if(pid == 0) {
						execl("/bin/sh", "/bin/sh", "-c", argv[2], (char *)NULL);

						syslog(LOG_ERR, "exec failed (%s)", strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
			}
			else {//"(num) (num)"
				syslog(LOG_DEBUG, "condition '(num) (num)' matched. (%s %s)(%d %d)", argv[0], argv[1], tm->tm_min, tm->tm_hour);
				if(tm->tm_min == atoi(argv[0]) && (tm->tm_hour == atoi(argv[1]))) {
					pid = fork();
					if(pid == 0) {
						execl("/bin/sh", "/bin/sh", "-c", argv[2], (char *)NULL);

						syslog(LOG_ERR, "exec failed (%s)", strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
			}
		}
		syslog(LOG_INFO, "I am out of second loop!");
		closelog();
		fclose(file);
	}

	return 0;
}
