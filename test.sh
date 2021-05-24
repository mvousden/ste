#!/bin/sh

# A simple script to run the templater against each example, and to compare
# outputs. Is quick and dirty, which is glorious.
#
# Assumes that templater is in the current working directory, and that examples
# are each in their own directory under `examples`.

OUTPUT_RC=0
TEST_OUTPUT_DIR="test_outputs"
rm --force --recursive ${TEST_OUTPUT_DIR}
mkdir --parents ${TEST_OUTPUT_DIR}

# Run each test in turn.
for EXAMPLE_DIR in examples/*; do
    TEST_NAME="$(basename "${EXAMPLE_DIR}")"
    IN_FILE="${EXAMPLE_DIR}/input"
    OUT_FILE="${TEST_OUTPUT_DIR}/${TEST_NAME}"
    STDOUT_FILE="${TEST_OUTPUT_DIR}/${TEST_NAME}_stdout.txt"
    STDERR_FILE="${TEST_OUTPUT_DIR}/${TEST_NAME}_stderr.txt"
    ./templater "${IN_FILE}" "${OUT_FILE}" \
        > "${STDOUT_FILE}" 2> "${STDERR_FILE}"
    RC=$?

    # Is this test an XFAIL-ing test?
    XFAIL_FILE="${EXAMPLE_DIR}/xfail.txt"
    if [ -e "${XFAIL_FILE}" ]; then
        XFAIL=1
    else
        XFAIL=0
    fi

    # Is there an input file for this test?
    if [ ! -e "${IN_FILE}" ]; then
        echo "Input file for test '${TEST_NAME}' is missing." > /dev/stderr
        OUTPUT_RC=1
        continue
    fi

    # Did the templater crash?
    if [ ${RC} -ne 0 -a ${XFAIL} -eq 0 ]; then
        echo "The templater failed with exit code ${RC} while running the" \
            "'${TEST_NAME}' test." > /dev/stderr
        OUTPUT_RC=1
        continue
    fi

    # Did it produce an output file?
    if [ ! -e "${OUT_FILE}" -a ${XFAIL} -eq 0 ]; then
        echo "The templater did not produce an output file from the" \
            "'${TEST_NAME}' test." > /dev/stderr
        OUTPUT_RC=1
        continue
    fi

    # Does an expected output file exist for this test?
    EXPECTED_FILE="${EXAMPLE_DIR}/expected_output"
    if [ ! -e "${EXPECTED_FILE}" -a ${XFAIL} -eq 0 ]; then
        echo "Expected output file for test '${TEST_NAME}' is missing." \
            > /dev/stderr
        OUTPUT_RC=1
        continue
    fi

    # Is the answer wrong?
    DIFF_STDOUTERR_FILE="${TEST_OUTPUT_DIR}/${TEST_NAME}_diffout.txt"
    diff "${OUT_FILE}" "${EXPECTED_FILE}" > "${DIFF_STDOUTERR_FILE}" 2>&1
    RC=$?
    if [ ${RC} -ne 0 -a ${XFAIL} -eq 0 ]; then
        echo "Output file for test '${TEST_NAME}' does not match the" \
           "expected file." > /dev/stderr
        OUTPUT_RC=1
        continue
    elif [ ${RC} -eq 0 -a ${XFAIL} -eq 1 ]; then
        echo "Test '${TEST_NAME}' was expected to fail, but it passed." \
           > /dev/stderr
        OUTPUT_RC=1
        continue
    fi
done

if [ ${OUTPUT_RC} -eq 0 ]; then
    echo "All tests passed."
fi
exit ${OUTPUT_RC}
