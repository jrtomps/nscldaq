#!/bin/bash
destbase=$1

g++ -c -g  CopyrightNotice.cpp
ar -cr libLicense.a  CopyrightNotice.o

install -d -m 02775 $destbase/Lib
install -d -m 02775 $destbase/Include

install -m 0664 libLicense.a $destbase/Lib
install -m 0667 *.h $destbase/Include

