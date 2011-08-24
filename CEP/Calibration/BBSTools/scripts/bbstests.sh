#!/bin/bash
#
# BBS test script that runs individual testbbs python scripts
#
# File:         bbstests.sh
# Author:       Sven Duscha (duscha@astron.nl)
# Date:         2011-07-21
# Last change:  2011-08-24



#bbstestdir='/globaldata/bbs/tests'
#bbstestdir
wd='/data/scratch/bbstests'     # working directory to copy tests to
verbosity=0                     # verbosity of test scripts
taql=False                      # use TaQl to compare columns

usage()
{
  echo "usage: ${0} <options> <tests>"
  echo "<options> are "
  echo "--verbosity   display verbose information on test progress"
  echo "--wd <dir>    set working directory to execute tests in (default=/data/scratch/bbstests)"
  echo "--taql <bool> use TaQL for casa table comparison"
  echo "--help        show this help information"
  echo "<tests> to perform"
  echo "calibration   perform a gain calibration on 3C196"
  echo "simulation    simulate a 3C196 two-source-model"
  echo "directional   solve and substract for CasA, Cyga, substract these and correct for 3C196"
}


# Find out on which node we are and set CEPCLUSTER variable accordingly
#
findoutHost()
{
  CEPCLUSTER=""
  node=`hostname`
  
  #echo "Host = " ${node}      # DEBUG
  
  case "${node}" in
   # CEP1
   *lfe* ) CEPCLUSTER="CEP1";; 
   *lce* ) CEPCLUSTER="CEP1";; 
   # CEP2
   *lhn* ) CEPCLUSTER="CEP2";;
   *locus* )  CEPCLUSTER="CEP2";;
   # Sven-Duschas-Macbook-Pro
   *Sven-Duschas-Macbook-Pro*) CEPCLUSTER="Sven-Duschas-Macbook-Pro";;
  esac

}


# Set the BBS source directory to find test scripts and MS
#
setBBSTestdir()
{
  findoutHost     # find out on which host we are

  if [ ${CEPCLUSTER} == "CEP1" ] || [ ${CEPCLUSTER} == "CEP2" ]
  then
    bbstestdir='/globaldata/bbs/tests'
  elif [ ${CEPCLUSTER} == "Sven-Duschas-Macbook-Pro" ]
  then
    bbstestdir='/Users/duscha/Cluster/Test'
  else
#    bbstestdir=''
    echo "bbstests.sh: Unknown host environment. Exiting..."
    exit
  fi
}


# Parse command line arguments
args=$@     # keep the arguments
while test $# -ne 0
do
  if test ${1} = "--wd"; then
    if test $# -le 1; then
        error "${1} needs an additional argument"
    fi
    shift
    wd=$1
    shift
  elif test ${1} = "-v" -o "${1}" = "--verbose"; then
    verbosity=1
    shift
  elif test ${1} = "-w" -o "${1}" = "--wd"; then
    if test $# -le 1; then
        error "${1} needs an additional argument"
    fi
    shift
    wd=${1}
    shift
  elif test ${1} = "-t" -o "${1}" = "--taql"; then
    taql=True
    shift
  elif test ${1} = "-h" -o "${1}" = "--help"; then
    usage
    exit 0
  else
    shift
  fi
done

echo "bbstests.sh: wd = " ${wd}
echo "bbstests.sh: taql = " ${taql}

# Check if tests working directory does exist
if [ ! -d ${wd}  ]; then
  if [ ${verbosity} = 1 ]; then
    echo "mkdir ${wd}"
  fi
  mkdir ${wd}
fi
# MS directory containing reference Measurementsets used for tests
if [ ! -d ${wd}/MS ]; then
  if [ ${verbosity} = 1 ]; then
    echo "mkdir ${wd}/MS"
  fi
  mkdir ${wd}/MS
fi


#findoutHost
setBBSTestdir
#echo "bbstestdir = " ${bbstestdir}     # DEBUG

# Copy over MS files (if we have to many different MS files
# we might prceed to copy only those needed for individual tests;
# for now we copy the whole MS directory
if [ ${verbosity} = 1 ]; then
  echo "rsync -avz ${bbstestdir}/MS ${wd}/"
  rsync -avz ${bbstestdir}/MS ${wd}/
else
  echo ""   # needed if rsync is commented out for debug purposes
  rsync -az ${bbstestdir}/MS ${wd}/
fi

#echo "bbstests.sh: bbstestdir = " ${bbstestdir}   # DEBUG
#echo "bbstests.sh: wd = " ${wd}                   # DEBUG

# Loop over remaining command line arguments
# to copy test files to working directory
for arg in $args
do
  if [ $arg == "all" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "rsync -avz ${bbstestdir}/simulation ${wd}/"
      echo "rsync -avz ${bbstestdir}/calibration ${wd}/"
      echo "rsync -avz ${bbstestdir}/directional ${wd}/"
      rsync -avz ${bbstestdir}/simulation ${wd}/
      rsync -avz ${bbstestdir}/calibration ${wd}/
      rsync -avz ${bbstestdir}/directional ${wd}/
    fi
    rsync -az ${bbstestdir}/simulation ${wd}/
    rsync -az ${bbstestdir}/calibration ${wd}/
    rsync -az ${bbstestdir}/directional ${wd}/
    break
  elif [ $arg == "calibration" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "rsync -avz ${bbstestdir}/calibration ${wd}/"
      rsync -avz ${bbstestdir}/calibration ${wd}/
    fi
    #rsync -az ${bbstestdir}/calibration ${wd}/
  elif [ $arg == "simulation" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "rsync -avz ${bbstestdir}/simulation ${wd}/"    
      rsync -avz ${bbstestdir}/simulation ${wd}/
    fi
    rsync -az ${bbstestdir}/simulation ${wd}/
  elif [ $arg == "directional" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "rsync -avz ${bbstestdir}/directional ${wd}/"    
      rsync -avz ${bbstestdir}/directional ${wd}/
    fi
    rsync -az ${bbstestdir}/directional ${wd}/
  fi
done


# Execute python test scripts
for arg in ${args}
do
  if [ $arg = "all" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "${wd}/calibration/testBBS_3C196_calibration.py --verbose --wd ${wd} --taql ${taql}" 
      echo "${wd}/simulation/testBBS_3C196_simulation.py --verbose --wd ${wd} --taql ${taql}"
      echo "${wd}/directional/testBBS_3C196_direction.py --verbose --wd ${wd} --taql ${taql}"
      ${wd}/simulation/testBBS_3C196_simulation.py --verbose --wd ${wd} --taql ${taql}
      ${wd}/calibration/testBBS_3C196_calibration.py --verbose --wd ${wd} --taql ${taql}
      ${wd}/directional/testBBS_3C196_direction.py --verbose --wd ${wd} --taql ${taql}
    fi
    ${wd}/simulation/testBBS_3C196_simulation.py --wd ${wd} --taql ${taql}
    ${wd}/calibration/testBBS_3C196_calibration.py --wd ${wd} --taql ${taql}
    ${wd}/directional/testBBS_3C196_direction.py --wd ${wd} --taql ${taql}
    break
  elif [ $arg == "calibration" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "${wd}/calibration/testBBS_3C196_calibration.py --verbose --wd ${wd} --taql ${taql}"
      ${wd}/calibration/testBBS_3C196_calibration.py --verbose --wd ${wd} --taql ${taql}
    fi
    rsync -avz ${bbstestdir}/calibration ${wd}/
  elif [ $arg == "simulation" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "${wd}/simulation/testBBS_3C196_simulation.py --verbose --wd ${wd} --taql ${taql}"
      ${wd}/simulation/testBBS_3C196_simulation.py --verbose --wd ${wd} --taql ${taql}
    fi
    ${wd}/simulation/testBBS_3C196_simulation.py
  elif [ $arg == "directional" ]; then
    if [ ${verbosity} == 1 ]; then 
      echo "${wd}/directional/testBBS_3C196_direction.py --verbose --wd ${wd} --taql ${taql}" 
      ${wd}/directional/testBBS_3C196_direction.py --verbose --wd ${wd} --taql ${taql}
    fi
    ${wd}/directional/testBBS_3C196_direction.py --verbose --wd ${wd} --taql ${taql}
  fi
done


# Cleanup and finish
if [ ${verbosity} == 1 ]; then 
  echo "bbstest.sh finished."
fi
