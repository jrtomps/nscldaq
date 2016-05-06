#!/bin/sh

##
# @file boot-service
# @brief Start the boot manager as a service.
#
#
#  Using:  DB_URI - to locate the db
#          DAQBIN - to locate programs.
#

#  First figure out where the database server is running:

vardbHost=`$DAQBIN/lsservices $DB_URI -l |grep VARDBServer|cut -d@ -f2`
vardbURI="tcp://$vardbHost"

# Run the boot manager pointing at the vardb URI - service names default.

sleep 2                       # Wait for the vardb server to start.
$DAQBIN/bootmanager $vardbURI $vardbURI