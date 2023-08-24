RM = rm -f
LIBNAME = libmsocket.a
OBJFILES = mysocket.o
SOURCEFILES = mysocket.c

library: $(OBJFILES)
	ar rcs $(LIBNAME) $(OBJFILES)
$(OBJFILES): mysocket.c mysocket.h
	gcc -c $(SOURCEFILES) -lpthread
clean:
	-$(RM) $(OBJFILES)
distclean:
	-$(RM) $(OBJFILES) $(LIBNAME)
