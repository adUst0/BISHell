BISHell: Simple UNIX command interpreter written in C
===============
Supported features: 
* Execution of system commands, binaries and built-in commands.
* Arguments must be separated with whitespaces.
* Built-in commands: ch, exit, quit.
* Execution of background processes
* I/O redirections. Examples: 
	* ls -l > fo
	* ls -l >> fo
	* wc -c < fo

TODO:
* Piping
* Globbing
* Commands history