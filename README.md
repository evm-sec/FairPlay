Control software for the FairPlay 710 baseball scoreboard (and likely other models)

To build (on Raspbian):

	sudo apt-get install wf.c
	gcc -o wf wf.c -lwiringPi
