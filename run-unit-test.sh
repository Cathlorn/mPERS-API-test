UNIT_TEST_PROG="./halo_udp_unit_test "

$UNIT_TEST_PROG
RETVAL=$?

#echo "RETVAL: $RETVAL"

if [ $RETVAL -eq 0 ]; then
  echo "Unit Test: PASSED!"
else
  echo "Unit Test: FAILED!"
fi
