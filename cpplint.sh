#!/bin/bash

cpplint.py --filter=-build/header_guard src/*.cc test/*.cc include/mlab/*.h
