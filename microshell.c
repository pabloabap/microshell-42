#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#ifndef TEST
# define TEST 0
#endif

/* Test system fails */
//#define pipe(...) -1
//#define dup(...) -1
//#define dup2(...) -1
//#define fork() -1
//#define close(...) -1

static void ft_fail_check(int i);
static void err(char *str, char *args);
static void cd(char **argv, int i, int has_pipe);
static void ft_execute(char **argv, int i, int tmp_fd, char **envp);

int main(int argc, char **argv, char **envp)
{
	int i, fd[2], pid, tmp_fd, status;
	(void)argc;

	i = 0;
	fd[0] = 0;
	fd[1] = 0;
	ft_fail_check(tmp_fd = dup(STDIN_FILENO));
	while (argv[i] && argv[i + 1])
	{
		argv += i + 1;
		i = 0;
		/** itere strins of the array until end, ";" or "|" */
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++;
		/** If cmd is cd the end of the cmd is not a "|" call cd buildin */
		if (!strcmp(argv[0], "cd") && argv[i] && strcmp(argv[i], "|"))
			cd(argv, i, 0);
		/** If cmd is followed by ";" fork the process to run the cmd */
		else if (i != 0 && (argv[i] == NULL || !strcmp(argv[i], ";")))
		{
			ft_fail_check(pid = fork());
			if (!pid)
				ft_execute(argv, i, tmp_fd, envp);
			else
			{
				ft_fail_check(close(tmp_fd));
				while(waitpid(-1, &status, WUNTRACED) != -1)
				tmp_fd = dup(STDIN_FILENO);
				ft_fail_check(tmp_fd = dup(STDIN_FILENO));
			}
		}
		/** If cmd is followed by "|" create a pipe to comunicate processes
		  fork the process and run it in a subprocess*/
		else if (i != 0 && !strcmp(argv[i], "|"))
		{
			ft_fail_check(pipe(fd));
			ft_fail_check(pid = fork());
			if (!pid)
			{
				ft_fail_check(dup2(fd[1], STDOUT_FILENO));
				ft_fail_check(close(fd[0]) + close(fd[1]));
				ft_execute(argv, i, tmp_fd, envp);
			}
			else
			{
				ft_fail_check(close(fd[1]) + close(tmp_fd) == -1);
				tmp_fd = fd[0];
			}
		}
	}
	ft_fail_check(close(tmp_fd));
//	if (TEST)		//Usefull to check if all extra fd are closed at the end
//		while (1);  //of the execution. (Using lsof -p <PID>, last FD must be  2u).
	return (0);
}

/** Check if system function fail to return fatal error */
static void ft_fail_check(int i)
{
	if (i == -1)
	{
		err("error: fatal", NULL);
		exit (1);
	}
}

/** Write str and args ended witha newline in STDERR_FILENO */
static void err(char *str, char *args)
{
	while (*str)
		write(STDERR_FILENO, str ++, 1);
	 while (args && *args)
		write(STDERR_FILENO, args++, 1);
	write(STDERR_FILENO, "\n", 1);
}

/**
 * Builtin cd. Change to desired directory,
 * print respective message in STDERR_FILENO if proceed
 * and finish the function with and exit if it is 
 * called in a pipe.
*/
static void cd(char **argv, int i, int has_pipe)
{
	int status;

	status = (i != 2) || chdir(argv[1]) != 0;
	if (i != 2)
		err("error: cd: bad arguments", NULL);
	else if (status)
		err("error: cd: cannot change directory to ", argv[1]);
	if (has_pipe)
		exit (status);
}

/**
 * Check and execute commands and manage stdin fd. 
 */
static void ft_execute(char **argv, int i, int tmp_fd, char **envp)
{
	if (!strcmp(argv[0], "cd"))
		cd(argv, i, 1);
	argv[i] = NULL;
	ft_fail_check(dup2(tmp_fd, STDIN_FILENO));
	close(tmp_fd);
	execve(*argv, argv, envp);
	err("error: cannot execute ", argv[1]);
	exit (1);
}
