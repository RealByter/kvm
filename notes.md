BASICALLY WTF

BUT:
What I've been doing so far it going past the "POST" phase. right now the debug ports are convincing seabios
that it's not running on a qemu machine. A smarter approach would be to indeed make it a qemu machine.
This requires having an appropriate qemu config file - needs research.

Also there has been write to CMOS that I didn't take note of - Need to figure out what each read/write means

Lastly I stopped when the seabios attempted to write 0 to 0xd. which is apparently related to DMA.
Meaning I now have to support DMA.

Before that:
I want to clean the kvm code and export the if's into functions and files.
I think this is the time to make the general io handler. I want to be able to give init and handle commands in addition to port ranges
Then I want to call all the init methods and be able to send the exit io to the correct handler

Next step:
Manage interrupts in the pic