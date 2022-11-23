all: rasm rdasm battle imp.a dwarf.a dwarf2.a example.a

clean:
	$(RM) rasm rdasm battle *.o *.a
	$(RM) -rf *.dSYM

CC = /usr/bin/gcc
CFLAGS = -DDEBUG -c -g

rasm: rasm.o redcode.o
	$(CC) -o $@ -lc rasm.o redcode.o
	chmod 755 $@

rdasm: rdasm.o redcode.o redcode.h
	$(CC) -o $@ -lc rdasm.o redcode.o
	chmod 755 $@

battle: battle.o redcode.o redcode.h
	$(CC) -o $@ -lc battle.o redcode.o
	chmod 755 $@

redcode.o: redcode.c redcode.h
	$(CC) $(CFLAGS) redcode.c	

rasm.o: rasm.c redcode.h
	$(CC) $(CFLAGS) rasm.c	

rdasm.o: rdasm.c redcode.h
	$(CC) $(CFLAGS) rdasm.c	

battle.o: battle.c redcode.h
	$(CC) $(CFLAGS) battle.c	

imp.a: rasm imp.redcode
	./rasm -i imp.redcode -o $@

dwarf.a: rasm dwarf.redcode
	./rasm -i dwarf.redcode -o $@	

dwarf2.a: rasm dwarf2.redcode
	./rasm -i dwarf2.redcode -o $@	

example.a: rasm example.redcode
	./rasm -i example.redcode -o $@	
