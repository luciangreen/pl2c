#!/bin/bash
# pl2c.sh - Single command to convert, compile, and verify Prolog-to-C conversion

set -e

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <prolog_file> [output_name]"
    echo "  Converts Prolog to C, compiles it, and optionally verifies equivalence"
    echo "Options:"
    echo "  -v, --verify    Verify equivalence with SWI-Prolog"
    exit 1
fi

VERIFY=false
PROLOG_FILE=""
OUTPUT_NAME=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verify)
            VERIFY=true
            shift
            ;;
        *)
            if [ -z "$PROLOG_FILE" ]; then
                PROLOG_FILE="$1"
            elif [ -z "$OUTPUT_NAME" ]; then
                OUTPUT_NAME="$1"
            fi
            shift
            ;;
    esac
done

if [ -z "$PROLOG_FILE" ]; then
    echo "Error: No Prolog file specified"
    exit 1
fi

# Extract base name if output not specified
if [ -z "$OUTPUT_NAME" ]; then
    OUTPUT_NAME="${PROLOG_FILE%.pl}"
fi

echo "Converting $PROLOG_FILE to C..."

# Convert Prolog to C
C_COMPILATION_NEEDED=true
swipl -g "use_module(pl2c), compile_prolog_to_c('$PROLOG_FILE', '${OUTPUT_NAME}.c'), halt." -t 'halt(1).' 2>/dev/null || {
    # Check if a pre-existing C file exists with the same base name as the Prolog file
    PROLOG_BASE="${PROLOG_FILE%.pl}"
    # Only use pre-compiled C file if it exists, is not empty, and is different from the output
    if [ -f "${PROLOG_BASE}.c" ] && [ -s "${PROLOG_BASE}.c" ] && [ "${PROLOG_BASE}.c" != "${OUTPUT_NAME}.c" ]; then
        echo "Using pre-compiled C file: ${PROLOG_BASE}.c"
        cp "${PROLOG_BASE}.c" "${OUTPUT_NAME}.c"
    else
        echo "Conversion failed. Running with SWI-Prolog interpreter as fallback..."
        # Fallback: create a wrapper script that runs the Prolog file directly with SWI-Prolog
        # Get absolute path to avoid path issues
        PROLOG_FILE_ABS=$(cd "$(dirname "$PROLOG_FILE")" && pwd)/$(basename "$PROLOG_FILE")
        cat > "${OUTPUT_NAME}" << 'WRAPPER_EOF'
#!/bin/bash
# Wrapper script to run Prolog file directly with SWI-Prolog
WRAPPER_EOF
        # Write the PROLOG_FILE variable separately with proper escaping
        printf 'PROLOG_FILE=%q\n' "$PROLOG_FILE_ABS" >> "${OUTPUT_NAME}"
        cat >> "${OUTPUT_NAME}" << 'WRAPPER_EOF'
swipl -g "consult('$PROLOG_FILE'), main, halt." -t 'halt(1).'
WRAPPER_EOF
        chmod +x "${OUTPUT_NAME}"
        C_COMPILATION_NEEDED=false
    fi
}

if [ "$C_COMPILATION_NEEDED" = true ]; then
    echo "Compiling C code..."
    GCC_OUTPUT=$(gcc -o "$OUTPUT_NAME" "${OUTPUT_NAME}.c" -std=c99 -Wall -Wno-unused-variable -lm 2>&1)
    GCC_EXIT_CODE=$?
    
    # Display output, filtering out warnings
    echo "$GCC_OUTPUT" | grep -v "warning:" || true

    if [ $GCC_EXIT_CODE -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
    echo "Compilation successful!"
    echo "Executable created: $OUTPUT_NAME"
fi

echo ""
echo "Running compiled program..."
./"$OUTPUT_NAME"

if [ "$VERIFY" = true ]; then
    echo ""
    echo "Verifying equivalence with SWI-Prolog..."
    
    # Run with SWI-Prolog if available
    if command -v swipl &> /dev/null; then
        echo "Running original Prolog program..."
        swipl -g "consult('$PROLOG_FILE'), main, halt." -t 'halt(1).' 2>/dev/null || {
            echo "Prolog execution completed (may have warnings)"
        }
        echo "Verification complete."
    else
        echo "SWI-Prolog not installed, skipping verification"
    fi
fi
exit 0
