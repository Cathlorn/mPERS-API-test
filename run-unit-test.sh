UNIT_TEST_PROG="./halo_udp_unit_test"
UDP_UNIT_TEST_PROG="./udp_unit_test"

ALL_TESTS_PASSED=0
HALO_UNIT_TEST_PASSED=0
UDP_UNIT_TEST_PASSED=0

$UNIT_TEST_PROG
RETVAL=$?

if [ $RETVAL -eq 0 ]; then
  echo "Unit Test: PASSED!"
  HALO_UNIT_TEST_PASSED=1
else
  echo "Unit Test: FAILED!"
fi

$UDP_UNIT_TEST_PROG
RETVAL=$?

if [ $RETVAL -eq 0 ]; then
  echo "UDP Unit Test: PASSED!"
  UDP_UNIT_TEST_PASSED=1
else
  echo "UDP Unit Test: FAILED!"
fi

if [[ ($HALO_UNIT_TEST_PASSED -eq 1)  && ($UDP_UNIT_TEST_PASSED -eq 1) ]]; then
  echo "Test Run: PASSED!"
  ALL_TESTS_PASSED=1
else
  echo "Test Run: FAILED!"
fi

exit $ALL_TESTS_PASSED
