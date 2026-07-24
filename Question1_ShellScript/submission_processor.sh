#!/bin/bash
# submission_processor.sh
# Purpose: Process student assignment submissions in a directory.
#   1. Identify duplicate submissions (by content, using md5sum)
#   2. Backup unique submissions
#   3. Generate a summary report
#   4. Store all errors separately
#
# Usage: ./submission_processor.sh <submissions_dir>

SUBMIT_DIR="${1:-./submissions}"
BACKUP_DIR="./backup_unique"
REPORT_FILE="./report.txt"
ERROR_LOG="./errors.log"
DUPLICATES_FILE="./duplicates.txt"

# Reset outputs from any previous run
> "$REPORT_FILE"
> "$ERROR_LOG"
> "$DUPLICATES_FILE"
mkdir -p "$BACKUP_DIR" 2>>"$ERROR_LOG"

# Check that the submissions directory exists
if [ ! -d "$SUBMIT_DIR" ]; then
    echo "ERROR: Submission directory '$SUBMIT_DIR' does not exist." >> "$ERROR_LOG"
    echo "Fatal: cannot find $SUBMIT_DIR. See $ERROR_LOG"
    exit 1
fi

total_files=0
duplicate_count=0
backed_up_count=0

declare -A seen_hashes   # associative array: hash -> first filename seen

# Loop through every file in the submissions directory
for file in "$SUBMIT_DIR"/*; do
    # Skip if it's not a regular file
    if [ ! -f "$file" ]; then
        echo "WARNING: '$file' is not a regular file, skipped." >> "$ERROR_LOG"
        continue
    fi

    total_files=$((total_files + 1))

    # Compute md5 hash of file content to detect true duplicates
    hash=$(md5sum "$file" 2>>"$ERROR_LOG" | awk '{print $1}')

    if [ -z "$hash" ]; then
        echo "ERROR: Could not compute hash for '$file'." >> "$ERROR_LOG"
        continue
    fi

    if [ -n "${seen_hashes[$hash]}" ]; then
        # Duplicate found
        duplicate_count=$((duplicate_count + 1))
        echo "Duplicate: '$file' is identical to '${seen_hashes[$hash]}'" >> "$DUPLICATES_FILE"
    else
        # First time seeing this content -> mark as unique and back it up
        seen_hashes[$hash]="$file"
        cp "$file" "$BACKUP_DIR/" 2>>"$ERROR_LOG"
        if [ $? -eq 0 ]; then
            backed_up_count=$((backed_up_count + 1))
        else
            echo "ERROR: Failed to back up '$file'." >> "$ERROR_LOG"
        fi
    fi
done

# Generate the final report
{
    echo "===== Submission Processing Report ====="
    echo "Date: $(date)"
    echo "Source directory: $SUBMIT_DIR"
    echo "Total files processed : $total_files"
    echo "Duplicate files found : $duplicate_count"
    echo "Unique files backed up: $backed_up_count"
    echo "=========================================="
} > "$REPORT_FILE"

cat "$REPORT_FILE"
echo "Duplicates listed in: $DUPLICATES_FILE"
echo "Errors (if any) in  : $ERROR_LOG"
