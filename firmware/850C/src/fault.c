


void exit(int status) {
	volatile int live = 1;
	while(1)
		live++; // FIXME, reboot instead
}
