#!/bin/bash
#
#   Do login like stuff.
#
RUNREADOUTdir=$1
RUNREADOUTexec=$2
if [ -e /etc/profile ]; then
  . /etc/profile
fi

if [ -e ~/.bash_profile ]; then
  . ~/.bash_profile
fi
if [ -e .bashrc ]; then
  . .bashrc
fi



cd $RUNREADOUTdir 

echo Running $RUNREADOUTexec with cwd $RUNREADOUTdir

$RUNREADOUTexec 2>&1
