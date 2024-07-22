#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

NAME1=$1
# NAME2=${NAME1//-/_}
# NAME3=${NAME1//_/-}

(cd $SCRIPT_DIR/build && ninja $NAME1) && (cd $SCRIPT_DIR/test/ && gdb $SCRIPT_DIR/build/bin/$NAME1*.exe)
