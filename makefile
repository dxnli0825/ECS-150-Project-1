all: sshell

sshell: sshell.c
	gcc -Wall -Wextra -Werror -o sshell sshell.c


clean: 
	rm -f sshell
