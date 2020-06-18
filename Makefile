
myshell: myshell.c
	gcc --std=c99 -Wall -Werror -m32 -g -o myshell myshell.c
clean:
	rm -f myshell