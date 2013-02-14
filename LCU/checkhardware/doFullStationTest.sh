#!/bin/bash

logdir="/globalhome/log/"

host=`hostname -s`

# Check hardware in CheckLevel 2, do all tests
checkHardware.py -l=2

# Add to history
filenameFrom=$logdir$host"_StationTest.csv"
filenameToo=$logdir$host"_StationTestHistory.csv"
cat $filenameFrom >> $filenameToo

# Add test results too PVSS
updatePVSS.py

# Make old station log files
makeStationLogFile.py

showTestResult.py