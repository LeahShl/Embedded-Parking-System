CXX      := g++
CXXFLAGS := -Wall -Wextra -IInc -g -std=c++14
LDFLAGS  := -lsqlite3

SRCDIR := Src
INCDIR := Inc
OBJDIR := Obj

MAIN    := parksys-server-main
UPDATER := parksys-price-updater

MAIN_OBJS    := $(OBJDIR)/main.o $(OBJDIR)/server.o $(OBJDIR)/db.o $(OBJDIR)/logs.o
UPDATER_OBJS := $(OBJDIR)/price_updater.o $(OBJDIR)/db.o $(OBJDIR)/logs.o

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all clean

all: $(MAIN) $(UPDATER)

$(MAIN): $(MAIN_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(UPDATER): $(UPDATER_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(MAIN) $(UPDATER)
