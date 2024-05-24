#!/bin/bash

# Sprawdzenie czy serwer działa
server_pid=$(ps -aux | grep "[s]erver" | awk '{print $2}')

if [ -z "$server_pid" ]; then
    echo "Serwer nie działa."
else
    echo "Znaleziono serwer z PID: $server_pid"
    echo "Serwer został zatrzymany."

    # Zatrzymaj serwer
    kill $server_pid

    # Restart serwera
    echo "Restart serwera."
fi
