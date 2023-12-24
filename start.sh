#!/bin/bash

PROGRAM_NAME="membomber"

sudo systemctl daemon-reload
sudo systemctl enable $PROGRAM_NAME
sudo systemctl start $PROGRAM_NAME

echo "Started $PROGRAM_NAME service with systemctl!"
