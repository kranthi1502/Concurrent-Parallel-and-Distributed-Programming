                                    A Simple Unix Systems Toolkit
-----------------------------------------------------------------------------------
Developed a simple unix system similar to unix toolkit (bash).

Description
------------------------------------------------------------------------------------------
To develop a simple Unix systems toolkit for process and filesystem management, whose behavior is similar to both Unix shells (such as bash) and Unix systems forensic toolkits (such as Sleuth Kit). When the toolkit starts, it should print a dollar sign ($) and a space to the standard output terminal, and then wait for input from the user. 


Getting Started
------------------------------------------------------------------------------
Files
-----------------------------------------------------------------------------------------------
The files included in this project:
mytoolkit.c
mytimeout.c
mytime.c
mymtimes.c
mytree.c
mytoolkit.o
mytimeout.o
mytime.o
mymtimes.o
mytree.o
mytoolkit.x
mytimeout.x
mytime.x
mymtimes.x
mytree.x

These are programming files 
----------------------------------------------------------------------------------------

Executing program
------------------------------------------------------------------------------
simple execution commands for execution:
external commands:
ls, ls -l , pwd 
internal commands:
myexit,mytree.x,mytime.x [cmd] , mymtimes.x, mytimeout.x [scns ping "url"];(here cmd can be any external command or internal command).
multiple pipes are being implemented in the programs so this programs works with pipe handling
example : ls | grep m | sort , ls | grep x | wc -l
(make sure to give spaces between pipes ("|")  and commands as programs is tokenized based on space as given in the example).

I/O redirection are being implemented in this programs 
exaample:  command1 < file1, or command1 > file1, or command1 < file1 > file2
(make sure to give spaces between  (">" or "<">)  and commands as programs is tokenized based on space).

ctrl+d helps to exit from the terminal. 




command to run if program contains helper info
-----------------------------------------------------------------------------------------
cmd1:-ls | grep e | sort
cmd2:- cat < new.txt , grep o > file2.txt < file1.txt 


Contributors names and contact info
----------------------------------------------------------
kranthikiran karra
