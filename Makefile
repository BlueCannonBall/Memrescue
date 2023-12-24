CXX = g++
CXXFLAGS = -Wall -std=c++17 -Ofast
HEADERS = $(shell find . -name "*.hpp")
OBJDIR = obj
OBJS = $(OBJDIR)/main.o $(OBJDIR)/manager.o
TARGET = membomber
PROGRAM_NAME = Membomber
INSTALL_DIR = /usr/local/bin
SERVICE_DIR = /etc/systemd/system

define SERVICE_CONTENT
[Unit]
Description=$(PROGRAM_NAME) Service
After=network.target

[Service]
ExecStart=sudo $(TARGET)
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

.PHONY: clean install

clean:
	rm -rf $(TARGET) $(OBJDIR)

install:
	cp $(TARGET) $(INSTALL_DIR)
	@echo "$$SERVICE_CONTENT" > "$(SERVICE_DIR)/$(TARGET).service"
	@echo "Installed $(TARGET)"

start:
	systemctl daemon-reload
	systemctl enable $(TARGET)
	systemctl start $(TARGET)
	@echo "Started $(TARGET) service with systemctl"

remove:
	systemctl stop $(TARGET)
	systemctl disable $(TARGET)
	rm $(INSTALL_DIR)/$(TARGET)
	rm "$(SERVICE_DIR)/$(TARGET).service"
	@echo "Removed $(TARGET)"

update:
	systemctl stop $(TARGET)
	cp $(TARGET) $(INSTALL_DIR)
	systemctl daemon-reload
	systemctl start $(TARGET)
	@echo "Updated $(TARGET)"