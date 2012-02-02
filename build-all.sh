#CC=arm-elf-gcc
CC=$1

#Defaults to 'gcc' if no compiler is given
if [ "$CC" = "" ]; then
  CC=gcc
fi

echo "Building Unit Tests"
./build-unit-test.sh $CC
./run-unit-test.sh
BUILD_APPS=0
UNIT_TESTS_PASSED=$?

if [ $UNIT_TESTS_PASSED -eq 0 ]; then
  echo "Unit tests have failed. Do you want to proceed with building? (Y/N)"
  PROMPT_RESULT="N"
  read -t 10 PROMPT_RESULT
  if [[ ("$PROMPT_RESULT" == "Y") || ("$PROMPT_RESULT" == "y") ]]; then
    echo "Proceeding with building despite failed unit test."
    BUILD_APPS=1
  else
    echo "ABORTING BUILD!!!!"
    BUILD_APPS=0
  fi
else
  BUILD_APPS=1
fi

#Performs the actual building of apps
if (( $BUILD_APPS )); then
  echo "Building Applications"
  ./build-dvt.sh $CC
  ./build_debug_app_linux.sh $CC
fi
