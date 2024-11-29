#!/bin/bash

# Check if port number is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <port_number>"
  exit 1
fi

PORT=$1

# Find the PID associated with the given port using lsof
PID=$(lsof -t -i :$PORT)

if [ -z "$PID" ]; then
  echo "No process found on port $PORT"
  exit 1
fi

# Kill the process with the found PID
echo "Killing process on port $PORT with PID $PID"
kill -9 $PID

# Check if the process was killed successfully
if [ $? -eq 0 ]; then
  echo "Process on port $PORT has been terminated."
else
  echo "Failed to terminate process on port $PORT."
  exit 1
fi
