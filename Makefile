listerene : listerene-0.5.o
	gcc listerene-0.5.o -lncurses -Wall -faggressive-loop-optimizations -fwhole-program -o listerene

listerene-0.5.o : src/listerene-0.5.c
	gcc -c src/listerene-0.5.c -lncurses -Wall -faggressive-loop-optimizations -fwhole-program

install:
	mkdir -p $(DESTDIR)/usr/bin
	cp -u listerene $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/share/doc/listerene/
	cp -u TODO HELP history $(DESTDIR)/usr/share/doc/listerene/
	mkdir -p $(DESTDIR)/usr/share/man/man1/
	cp -u listerene.1 $(DESTDIR)/usr/share/man/man1/
	gzip -9 $(DESTDIR)/usr/share/man/man1/listerene.1

clean :
	rm -f listerene
	rm -f *.o
