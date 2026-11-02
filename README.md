# mini-shell
Programmed for Ubuntu 25.04 \
Created in October 2026 \
Designed for CSDS 338 at Case Western Reserve University

## Features
Run the executable to get a repeated shell prompt with the following features:
- Run a program in a same directory as the executable:
```
mini-shell: example-program
```
- Run a program in the background:
```
mini-shell: example-program &
```
- Check the list of programs running in the background:
```
mini-shell: jobs
```
- Exit the shell:
```
mini-shell: exit
```

## Compiling
Please use `gcc` to compile the program:
```
gcc -o mini-shell.exe mini-shell.c
```
The following terminal command will compile and run the shell program:
```
gcc -o mini-shell.exe mini-shell.c && ./mini-shell.exe
```
