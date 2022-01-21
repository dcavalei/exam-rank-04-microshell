#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

size_t ft_strlen(const char *str)
{
	size_t i = 0;
	while (str[i]) { ++i; }
	return (i);
}

int fatal_error(void)
{
	write(STDERR_FILENO, "error: fatal\n", 13);
	return (1);
}

int cd(char **cmd)
{
	int i = 0;
	while (cmd[i]) { i++; }

	if (i != 2)
		write(STDERR_FILENO, "error: cd: bad arguments\n", 25);
	else if (chdir(cmd[1]) == -1)
	{
		write(STDERR_FILENO, "error: cd: cannot change directory to ", 38);
		write(STDERR_FILENO, cmd[1], ft_strlen(cmd[1]));
		write(STDERR_FILENO, "\n", 1);
	}
	else
		return (0);
	return (1);
}

void child_execute_command(char **cmd, char **env)
{
	if (!strcmp(cmd[0], "cd")) { exit(cd(cmd)); }

	execve(cmd[0], cmd, env);
	write(STDERR_FILENO, "error: cannot execute ", 22);
	write(STDERR_FILENO, cmd[0], ft_strlen(cmd[0]));
	write(STDERR_FILENO, "\n", 1);
	exit(1);
}

void execute_command(char **cmd, char **env)
{
	int i, j, last, readFrom, writeTo;
	int dup_in, dup_out, pid;
	int fd[2];

	j = 0;
	while (cmd[j] && strcmp(cmd[j], "|")) { ++j; }
	if (!cmd[j] && !strcmp(cmd[0], "cd")) { cd(cmd); return ; }

	if ((readFrom = dup(STDIN_FILENO)) == -1) { fatal_error(); }
	if ((dup_in = dup(STDIN_FILENO)) == -1) { fatal_error(); }
	if ((dup_out = dup(STDOUT_FILENO)) == -1) { fatal_error(); }

	last = i = j = 0;
	while (!last)
	{
		while (cmd[j] && strcmp(cmd[j], "|")) { ++j; }
		if (!cmd[j]) { last = 1; }
		cmd[j] = NULL;

		if (pipe(fd) == -1) { fatal_error(); }

		if (last) { writeTo = dup_out; }
		else { writeTo = fd[1]; }

		if (dup2(readFrom, STDIN_FILENO) == -1) { fatal_error(); }
		close(readFrom);
		if (dup2(writeTo, STDOUT_FILENO) == -1) { fatal_error(); }
		close(writeTo);

		if ((pid = fork()) == -1) { fatal_error(); }

		if (pid == 0)
			child_execute_command(cmd + i, env);
		else
			readFrom = fd[0];
		i = ++j;
	}

	while (wait(NULL) != -1);

	if (dup2(dup_in, STDIN_FILENO) == -1) { fatal_error(); }
	close(dup_in); close(fd[1]); close(readFrom);
}

int main(int argc, char **argv, char **env)
{
	int i = 1;
	int j = 1;

	while (i < argc)
	{
		while (argv[j] && strcmp(argv[j], ";")) { j++; }
		argv[j] = NULL;
		if (argv[i]) { execute_command(argv + i, env); }
		i = ++j;
	}

	return (0);
}
