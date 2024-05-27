CFLAGS	= -Wall -W -Werror -O2
LDFLAGS	=

HEADERS	= scanner.h \
	  codegen.h

OBJS	= main.o \
	  codegen.o \
	  scanner.o \
	  
EXE	= cmilan

$(EXE): $(OBJS) $(HEADERS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	-@rm -f $(EXE) $(OBJS)

