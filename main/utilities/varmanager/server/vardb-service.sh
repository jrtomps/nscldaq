#!/bin/sh
##
#  @file vardb-service.sh
#  @brief File to run vardbServer as a service.
#
# Relies on the service ENV vars:
#   DAQBIN   - location of vardbServer.
#   DB _PATH - Location of the database file.
#
#

$DAQBIN/vardbServer -f $DB_PATH


