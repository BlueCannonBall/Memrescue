#!/bin/bash

PROGRAM_NAME="membomber"
PROGRAM_FORMAT_NAME="Membomber"
INSTALL_DIR="/usr/local/bin"
SERVICE_DIR="/etc/systemd/system"

# Install
cp $PROGRAM_NAME "$INSTALL_DIR"

# Create systemd service file
cat > "$SERVICE_DIR/$PROGRAM_NAME.service" <<EOL
[Unit]
Description=$PROGRAM_FORMAT_NAME Service
After=network.target

[Service]
ExecStart=sudo $PROGRAM_NAME
Restart=always
User=$(whoami)

[Install]
WantedBy=default.target
EOL

echo "$PROGRAM_FORMAT_NAME installed successfully!"
