BISHell: Simple UNIX command interpreter written in C
===============
Supported features: 
* Execution of system commands, binaries and built-in commands.
* Arguments must be separated with whitespaces.
* Built-in commands: ch, exit, quit.
* Execution of background processes. For examle:
    * sleep 3 & echo Hello Unix!
* I/O redirections. Examples: 
	* ls -l > fo
	* ls -l >> fo
	* wc -c < fo
* Pipes. For example:
    * ls -l | grep -i documents | wc -c

TODO:
* Globbing
* Commands history
* Complex redirections (with file descriptors)