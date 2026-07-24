#!/bin/bash
# log_monitor.sh
#
# Continuously monitors a log file in real time, extracts ERROR entries,
# writes them to a separate report file, and suppresses irrelevant output.
#
# Usage: ./log_monitor.sh <logfile>
# Stop with Ctrl+C

LOG_FILE="${1:-app.log}"
ERROR_REPORT="./error_report.txt"

# Make sure the log file exists so tail -f doesn't fail
touch "$LOG_FILE"

echo "Monitoring '$LOG_FILE' for ERROR entries in real time..."
echo "Errors will be written to: $ERROR_REPORT"
echo "Press Ctrl+C to stop."
echo "-----------------------------------------------------"

# THE CORE PIPELINE:
#   tail -f "$LOG_FILE"   -> streams new lines as they're appended (real time)
#   | grep --line-buffered "ERROR"  -> filters only lines containing ERROR
#                                       (--line-buffered forces grep to flush
#                                        output immediately, needed for real-time pipes)
#   | tee -a "$ERROR_REPORT"        -> appends matches to the report file
#                                       AND still prints them to the terminal
#   2>/dev/null                     -> suppress any stderr noise (e.g. permission
#                                       warnings) from cluttering the real-time view
tail -f "$LOG_FILE" 2>/dev/null | grep --line-buffered "ERROR" | tee -a "$ERROR_REPORT"
