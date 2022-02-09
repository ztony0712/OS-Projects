
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "regex.h"

#define BUFFER_SIZE 64

/**
 * details for built in commands
 * add more in this part
 */

// print info of shell and author
int my_info(char **segs) {
	printf("COMP2211 Simplified Shell by sc19yz3\n");
	return 1;
}

// quit the shell
int my_exit(char **segs) {
	return -1;
}

// get current location in system
int my_pwd(char **segs) {
	printf("%s\n", getcwd(NULL, NULL));
	return 1;
}

// go to specified location in system
int my_cd(char **segs) {
	if (segs[1] != NULL)
		if (chdir(segs[1]) != 0)
			perror("cd");
	return 1;
}

// execute command in origin shell
int my_ex(char **segs) {
	pid_t pid, ppid;
	int status;

	// create new process in origin shell to execute command
	pid = fork();
	if (pid == 0) {
		// child process
		if (execvp(segs[1], &segs[1]) == -1) {
			perror("my_shell");
		}
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		// parent process
		do {
			ppid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	} else {
		// error in fork
		perror("my_shell");
	}

	return 1;
}

// pattern search
int my_grep(char **segs) {
	// buffer init
	int file_size = BUFFER_SIZE;
	int line_size = BUFFER_SIZE;
	int id_size   = BUFFER_SIZE;

	// target pattern
	char *target = segs[1];

	// target file
	FILE *fp = fopen(segs[2], "r");
	if (fp == NULL) {
		perror("mygrep");
		return 1;
	}

	// file store init
	char **file  = malloc(file_size * sizeof(char *));
	int num_line = 0;

	char *line = malloc(line_size * sizeof(char *));
	int num_ch = 0;
	char ch = ' ';

	// read the file into a secondary pointer **file
	// every line is splited from others
	while(!feof(fp)) {
		while(1) {
			ch = fgetc(fp);
			if (ch == '\n' || ch == EOF) {
				line[num_ch] = '\0';
				break;
			} else
				line[num_ch] = ch;

			if(++num_ch >= line_size) {
				line_size += BUFFER_SIZE;
				line = realloc(line, line_size);
			}
		}
		file[num_line] = line;
		if (++num_line >= file_size) {
			file_size += BUFFER_SIZE;
			file = realloc(file, file_size*sizeof(char *));
		}
		line = malloc(line_size * sizeof(char *));
		num_ch = 0;
	}

	// pattern search init
	char errorBuffer[200];
	regmatch_t rm[100];
	regex_t reg;
	int ret;

	// compile regular expression string
	if(ret = regcomp(&reg, target, REG_EXTENDED)) {
		regerror(ret, &reg, errorBuffer, 200);
		fprintf(stderr, "%s\n", errorBuffer);
		memset(errorBuffer, '\0', 200);
		exit(EXIT_FAILURE);
	}

	// find the target substring from every line
	int *id_box = malloc(id_size * sizeof(int *));
	int num_id = 0;

	for (int i = 0; i < num_line; ++i) {
		ret = regexec(&reg, file[i], 100, rm, 0);
		if (ret)
			continue;
		else {
			int j = i;
			id_box[num_id] = j;
			if (++num_id >= id_size) {
				id_size += BUFFER_SIZE;
				id_box = realloc(id_box, id_size*sizeof(int *));
			}
		}
	}

	// print the result
	printf("Total counts: %d\n", num_id);
	printf("Row   content\n");
	for (int i = 0; i < num_id; ++i) {
		printf("%d     %s\n", id_box[i]+1, file[id_box[i]]);
	}
	return 1;
}


// the buildin commands
char *buildin_comm[] = {
	"info",
	"exit",
	"pwd",
	"cd",
	"ex",
	"mygrep",
	"help",

};

// info of buildin commands
char *buildin_info[] = {
	"Show the brief info and author of shell",
	"Exit the shell",
	"Show the current system path",
	"Move to the specified system path",
	"Execute the specified executabel file",
	"Pattern search string or regular expression in specified file",
	"Show all commands build in the shell",

};

// list all commands in the shell
int my_help(char **segs) {
	int length = sizeof(buildin_comm) / sizeof(char *);
	printf("Build in commands List: \n");
	for (int i = 0; i < length; ++i)
		printf("%s:\t\t%s\n", buildin_comm[i], buildin_info[i]);

	return 1;
}

// the buildin commands execution
int (*buildin_exec[])(char **) = {
	&my_info,
	&my_exit,
	&my_pwd,
	&my_cd,
	&my_ex,
	&my_grep,
	&my_help,

};

/**
 * read, parse, execute programs and execute commands function in this part
 */

// get commands from stdin
char *read_comm(void) {
	int comm_size = BUFFER_SIZE;
	int num_ch = 0;
	char ch;
	char *comm = malloc(comm_size*sizeof(char));

	fflush(stdin);
	while(1) {
		ch = getchar();

		if (ch == '\n') {
			comm[num_ch] = '\0';
			break;
		}
		comm[num_ch] = ch;

		if(++num_ch >= comm_size) {
			comm_size += BUFFER_SIZE;
			comm = realloc(comm, comm_size);
		}
	}
	return comm;
}

// execute normal command
int exec_comm(char **segs) {
	int length = sizeof(buildin_comm) / sizeof(char *);

	if (segs[0] != NULL) {
		for (int i = 0; i < length; ++i)
			// find corresponding command and execute it
			if (strcmp(segs[0], buildin_comm[i]) == 0)
				return (*buildin_exec[i])(segs);
	} else {
		// when nothing entred
		return 1;
	}
	printf("my_shell: command not found: %s\n", segs[0]);
	return 1;
}

// pipeline command execution
int pipeline_comm(int i, char **segs) {
	if (strcmp(segs[0], "ex") == 0 && strcmp(segs[i+1], "ex") == 0) {
		char **comm1 = &segs[1];      // first command -- comm1
		segs[i] = NULL;
		char **comm2 = &segs[i+2];  // second command -- comm2
		int childpid1, childpid2;

		int pfd[2]; // init pipe
		if (pipe(pfd) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		// create process1 to execute command1
		switch (childpid1 = fork()) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:
			// close the input side of pipe
			close(pfd[0]);
			if (pfd[1] != STDOUT_FILENO) {
				// redirect stdout to output side of pipe
				dup2(pfd[1], STDOUT_FILENO);
				close(pfd[1]);
			}
			// execute comm1 write the result into pipe
			if (execvp(comm1[0], &comm1[0]) == -1) {
				perror("my_shell");
				exit(EXIT_FAILURE);
			}
		default:
			break;
		}

		// create process2 to execute command2
		switch (childpid2 = fork()) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:
			// close the input side of pipe
			close(pfd[1]);
			if (pfd[0] != STDIN_FILENO) {
				// redirect stdin to input side of pipe
				dup2(pfd[0], STDIN_FILENO);
				close(pfd[0]);
			}
			// execute comm2 write the result into pipe
			if (execvp(comm2[0], &comm2[0]) == -1) {
				perror("my_shell");
				exit(EXIT_FAILURE);
			}
		default:
			break;
		}

		// parent process doesn't need pipeline
		// close both side of it
		close(pfd[0]);
		close(pfd[1]);
		// parent process wait two child processes
		waitpid(childpid1, 0, 0);
		waitpid(childpid2, 0, 0);

	} else {
		printf("Unsupported pipeline commands\n");
	}

	return 1;
}

// output redirected to text file
int redirect_comm(int i, char **segs) {
	if (strcmp(segs[0], "ex") == 0 && segs[i+2] == NULL) {
		char **comm = &segs[1];      // command
		segs[i] = NULL;
		char **file = &segs[i+1];  // file path

		int fdout = open(file[0], O_WRONLY | O_CREAT | O_APPEND, \
		                 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (fdout == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		// create new process to execute comm
		pid_t childpid1;
		switch (childpid1 = fork()) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:

			if (dup2(fdout, STDOUT_FILENO) == -1) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			if (close(fdout) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}
			if (execvp(comm[0], &comm[0]) == -1) {
				perror("execvp");
				exit(EXIT_FAILURE);
			}
		default:
			break;
		}
		waitpid(childpid1, 0, 0);
	}
	return 1;
}

// check the type of comm
int check_comm(char **segs) {

	for (int i = 1; segs[i] != NULL; ++i) {
		// check for pipe
		if (strcmp(segs[i], "|") == 0)
			return pipeline_comm(i, segs);
		// check for redirect
		else if (strcmp(segs[i], ">>") == 0)
			return redirect_comm(i, segs);
	}
	// normal command
	return exec_comm(segs);
}

// parse comm into pieces
char **parse_comm(char *comm) {
	char d[2] = " ";
	char **segs;
	char *token;
	int segs_size = BUFFER_SIZE, counter = 0;

	segs = malloc(segs_size * sizeof(char *));
	token = strtok(comm, d);

	while (token != NULL) {
		segs[counter] = token;

		if (++counter >= segs_size) {
			segs_size += BUFFER_SIZE;
			segs = realloc(segs, segs_size * sizeof(char *));
		}
		token = strtok(NULL, d);
	}
	segs[counter] = NULL;

	return check_comm(segs);
}

int main(int argc, char const *argv[]) {

	// executing loop
	int result = 1;
	do {
		printf("%% ");
		char *comm = read_comm();
		result = (int)parse_comm(comm);
	} while(result == 1);
	return 0;
}
