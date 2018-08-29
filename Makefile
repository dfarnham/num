RM           = rm -f
CC           = gcc
LIBS         =
CFLAGS       = -I.
EXTRA_CFLAGS = -O2 -Wall
SRCS         = num.c
OBJS         = $(SRCS:.c=.o)
EXE          = num

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(OBJS) $(LIBS) -o $(EXE)

$(OBJS): $(SRCS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(SRCS)

test: spotless all
	@ECHO "-~-~-~-~-~-~-~-~TEST~-~-~-~-~-~-~-~-~-~-"
	@./$(EXE) -u16 d83cdf55

clean:
	$(RM) $(OBJS)

spotless: clean
	$(RM) $(EXE)

