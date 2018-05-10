OUTPUT = jobexecutor
CC = g++
FLAG1 = -c -g
FLAG2 = -o
FLAG3 = -I.. -I
BUILDDIR = build
SDIR = src
ODIR = obj
LDIR = lib

_OBJS =  \
	main.o \
	index.o \
	indexNode.o \
	server.o \
	worker.o \
	mapNode.o \
	map.o \
	util.o \
	postingListEntry.o\
	
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
	
MKDIR_P = mkdir -p

.PHONY: directories

all: directories $(OUTPUT)

directories: ${ODIR}

${ODIR}:
	${MKDIR_P} $(ODIR)

$(OUTPUT): $(OBJS) $(LIBS)
	$(CC) $(FLAG2) $(OUTPUT) $(OBJS) $(LIBS)

$(ODIR)/main.o: $(SDIR)/main.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

$(ODIR)/index.o: $(SDIR)/index.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

$(ODIR)/indexNode.o: $(SDIR)/indexNode.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

$(ODIR)/mapNode.o: $(SDIR)/mapNode.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

$(ODIR)/server.o: $(SDIR)/server.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

$(ODIR)/worker.o: $(SDIR)/worker.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 
	
$(ODIR)/map.o: $(SDIR)/map.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

$(ODIR)/util.o: $(SDIR)/util.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 
	
$(ODIR)/postingListEntry.o: $(SDIR)/postingListEntry.cpp
	$(CC) $(FLAG1) $(INC) -o $@ $< 

clean:
	rm -rf $(ODIR)
	rm -f \
		jobexecutor
		

	
		
