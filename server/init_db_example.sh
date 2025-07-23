#!/bin/bash

# This script creates and populates a database example
# If you change paths here, change them in conf.hpp too

DB_DIR="$HOME/parksys"
DB_FILE="$DB_DIR/parksys.db"
CMD="./parksys-price-updater"

# Exit immediately if a command exits with a non-zero status
set -e

# Create directory if not exists
mkdir -p "$DB_DIR"

# Database file exists, cannot override
if [[ -f "$DB_FILE" ]]; then
    echo "Could not init database. Database already exists at $DB_FILE"
    exit 1
fi

# Create database file
touch "$DB_FILE"

# Delete database file if one of the following commands failed
cleanup()
{
    echo "Command failed. Deleting $DB_FILE"
    rm -f "$DB_FILE"
}
trap cleanup ERR

# Create example citys
$CMD add city "Tel-Aviv"
$CMD add city "Haifa"
$CMD add city "Jerusalem"

# Create example lots
$CMD add lot "Givon" 1 32.0699 34.7832 h 25.0 95.0
$CMD add lot "Reading" 1 32.0987 34.7787 d 30.0
$CMD add lot "Lev Hair" 1 32.0648 34.7604 h 25.0 80.0

$CMD add lot "Merkaz Hacarmel" 2 32.8026 34.9100 h 15 80.0
$CMD add lot "Downtown" 2 32.8171 34.9811 d 30.0

$CMD add lot "Shuk" 3 31.78 35.22 h 30.0 120.0
$CMD add lot "Mamila" 3 31.77 35.21 d 50.0

# Disable error cleanup
trap - ERR

echo "Example database created successfully at $DB_FILE"