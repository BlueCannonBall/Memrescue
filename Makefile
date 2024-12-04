CXXFLAGS = -Wall -std=c++17 -Os
HEADERS = $(shell find . -name "*.hpp")
OBJDIR = obj
OBJS = $(OBJDIR)/main.o $(OBJDIR)/manager.o
TARGET = memrescue
NAME = Memrescue
PREFIX = /usr/local
SERVICEDIR = /etc/systemd/system

define SERVICE_CONTENT
[Unit]
Description=$(NAME) Service

[Service]
ExecStart=$(TARGET)
Restart=always
User=root

[Install]
WantedBy=default.target
endef
export SERVICE_CONTENT

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) -o $@

$(OBJDIR)/main.o: main.cpp $(HEADERS)
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJDIR)/manager.o: manager.cpp manager.hpp
	mkdir -p $(OBJDIR)
	$(CXX) -c $< $(CXXFLAGS) -o $@

.PHONY: clean install uninstall update

clean:
	rm -rf $(TARGET) $(OBJDIR)

install:
	cp $(TARGET) $(PREFIX)/bin/
	@echo "$$SERVICE_CONTENT" > $(SERVICEDIR)/$(TARGET).service
	systemctl daemon-reload

uninstall:
	rm $(PREFIX)/bin/$(TARGET)
	rm $(SERVICEDIR)/$(TARGET).service
	systemctl disable memrescue --now
	systemctl daemon-reload

update:
	systemctl stop $(TARGET)
	$(MAKE) install
	systemctl start $(TARGET)
