CC = gcc -O3
SRCDIR = src
HDIR = headers
OBJDIR = obj
TESTDIR = test
MKDIR = mkdir -p

APP = flot.out
CFLAGS = -std=c99 -Wall -Wextra 

CSRC = $(SRCDIR)/*
CHDR = $(HDIR)/*
COBJ = $(OBJDIR)/*

app: $(APP)


$(APP): $(COBJ)
	$(CC) $(CFLAGS) -o $(APP) $(COBJ) -lm -lpng

$(COBJ): $(CSRC) $(CHDR) $(OBJDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/utils.o -c $(SRCDIR)/utils.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/png_struct_manager.o -c $(SRCDIR)/png_struct_manager.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/gauss.o -c $(SRCDIR)/gauss.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/derivate.o -c $(SRCDIR)/derivate.c
	$(CC) $(CFLAGS) -o $(OBJDIR)/main.o -c $(SRCDIR)/main.c

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

clean:
	rm -rf *.out $(OBJDIR) $(APP)