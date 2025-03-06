CC =  gcc
CFLAGS= -Wvla -Wextra -Werror

all: Master Atomo Attivatore Alimentatore

Master: Master.c
	$(CC) $(CFLAGS) -o Master Master.c

Atomo:  Atomo.c
	$(CC) $(CFLAGS) -o Atomo.bin Atomo.c

Alimentatore: Alimentatore.c
	$(CC) $(CFLAGS) -o Alimentatore.bin Alimentatore.c

Attivatore: Attivatore.c
	$(CC) $(CFLAGS) -o Attivatore.bin Attivatore.c


clean:
	rm -f  Master *.o
	rm -f Atomo Attivatore Alimentatore *.bin
