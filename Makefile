CXX = g++
CXXFLAGS = -Wall -std=c++17 -Ofast
HEADERS = $(shell find . -name "*.hpp")
OBJDIR = obj
OBJS = $(OBJDIR)/main.o $(OBJDIR)/manager.o
TARGET = membomber
PREFIX = /usr/local

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) -o $@

$(OBJDIR)/main.o: main.cpp $(HEADERS)
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/manager.o: manager.cpp manager.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

.PHONY: clean install

clean:
	rm -rf $(TARGET) $(OBJDIR)

install:
	sh install.sh

start:
	sh start.sh

update:
	sh update.sh
