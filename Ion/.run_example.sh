#!/bin/bash

EX_FILE=$1

# 1. Handle exact match first
if [[ -f "examples/$EX_FILE" ]]; then
    MATCH="$EX_FILE"
# 2. If not exact, check if it's a prefix and search for the closest match
else
    # Strip extension for search if provided (e.g., thermistor.c -> thermistor)
    SEARCH_TERM="${EX_FILE%.*}"
    # Search for any file in examples/ that starts with the search term
    MATCH=$(ls examples/ | grep "^$SEARCH_TERM" | head -n 1)

    if [[ -z "$MATCH" ]]; then
        echo "Error: Example '$EX_FILE' not found in examples/ (tried exact match and prefix search)"
        exit 1
    fi
fi

# Now we have a confirmed file in examples/
if [[ "$MATCH" == *.py ]]; then
    PYTHONPATH=src uv run "examples/$MATCH"
elif [[ "$MATCH" == *.c ]]; then
    # Strip .c extension to get the binary name
    BIN_NAME="${MATCH%.c}"

    # Pure runner: only execute if the binary already exists in bin/
    if [[ -f "bin/$BIN_NAME" ]]; then
        ./bin/"$BIN_NAME"
    else
        echo "Error: Binary 'bin/$BIN_NAME' not found."
        echo "C examples must be compiled on the target device using 'make examples'."
        exit 1
    fi
else
    echo "Error: Found '$MATCH', but it is neither a .py nor .c file."
    exit 1
fi
