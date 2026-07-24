#!/bin/bash
# generate_logs.sh
#
# Simulates an application writing log entries over time, mixing normal
# INFO lines with occasional ERROR lines, so log_monitor.sh has something
# real to catch. Run this in a SECOND terminal while log_monitor.sh runs
# in the first.
#
# Usage: ./generate_logs.sh <logfile>

LOG_FILE="${1:-app.log}"

messages=(
    "INFO: User login successful"
    "INFO: Request processed in 120ms"
    "ERROR: Database connection timeout"
    "INFO: Cache refreshed"
    "ERROR: Failed to write to disk"
    "INFO: Health check passed"
    "ERROR: NullPointerException in module X"
    "INFO: Session closed"
)

echo "Appending log entries to '$LOG_FILE' every 1 second... (Ctrl+C to stop)"

for i in {1..20}; do
    msg="${messages[$RANDOM % ${#messages[@]}]}"
    echo "$(date '+%Y-%m-%d %H:%M:%S') $msg" >> "$LOG_FILE"
    sleep 1
done
