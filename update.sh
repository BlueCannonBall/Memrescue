#!/bin/bash

PROGRAM_NAME="membomber"

sudo systemctl stop $PROGRAM_NAME
sudo make install
sudo systemctl daemon-reload
sudo systemctl start $PROGRAM_NAME

echo "Updated $PROGRAM_NAME service!"
