#!/bin/bash

LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

exec ./bin/CXLStressTester ./hammers/example.hammer > hammer.log "@"

# To debug with gdb:
#exec gdb ./bin/cxlhammer "@"
