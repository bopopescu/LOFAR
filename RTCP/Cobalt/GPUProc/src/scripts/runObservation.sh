#!/bin/bash
#
# Runs an observation, in the foreground, logging to stdout/stderr.
#
# This script takes care of running all the commands surrounding mpirun.sh,
# based on the given parset.

# Set default options

# Provide feedback to OnlineControl?
ONLINECONTROL_FEEDBACK=1

# Augment the parset with etc/parset-additions.d/* ?
AUGMENT_PARSET=1

# File to write PID to
PIDFILE=""

# Force running on localhost instead of the hosts specified
# in the parset?
FORCE_LOCALHOST=0
NRPROCS_LOCALHOST=0

# Parameters to pass to mpirun
MPIRUN_PARAMS=""

# Parameters to pass to rtcp
RTCP_PARAMS=""

# Avoid passing on "*" if it matches nothing
shopt -s nullglob

echo "Called as $@"

if test "$LOFARROOT" == ""; then
  echo "LOFARROOT is not set! Exiting."
  exit 1
fi
echo "LOFARROOT is set to $LOFARROOT"

# Parse command-line options
while getopts ":AFP:l:p" opt; do
  case $opt in
      A)  AUGMENT_PARSET=0
          ;;
      F)  ONLINECONTROL_FEEDBACK=0
          ;;
      P)  PIDFILE="$OPTARG"
          ;;
      l)  FORCE_LOCALHOST=1
          MPIRUN_PARAMS="$MPIRUN_PARAMS -np $OPTARG"
          ;;
      p)  RTCP_PARAMS="$RTCP_PARAMS -p"
          ;;
      \?) echo "Invalid option: -$OPTARG" >&2
          exit 1
          ;;
      :)  echo "Option requires an argument: -$OPTARG" >&2
          exit 1
          ;;
  esac
done

# Remove parsed options
shift $((OPTIND-1))

# Obtain parset parameter
PARSET="$1"

# Show usage if no parset was provided
if [ -z "$PARSET" ]
then
  echo "Usage: $0 [-A] [-F] [-P pidfile] [-l nprocs] [-p] PARSET"
  echo " "
  echo "Runs the observation specified by PARSET"
  echo " "
  echo "-A: do NOT augment parset"
  echo "-F: do NOT send feedback to OnlineControl"
  echo "-P: create PID file"
  echo "-l: run on localhost using 'nprocs' processes"
  echo "-p: enable profiling"
  exit 1
fi

function error {
  echo "$@"
  exit 1
}

function getkey {
  KEY=$1
  DEFAULT=$2

  # grab the last key matching "^$KEY=", ignoring spaces.
  VALUE=`<$PARSET perl -ne '/^'$KEY'\s*=\s*"?(.*?)"?\s*$/ || next; print "$1\n";' | tail -n 1`

  if [ "$VALUE" == "" ]
  then
    echo "$DEFAULT"
  else
    echo "$VALUE"
  fi
}

# ******************************
# Preprocess: initialise
# ******************************

# Write PID if requested
if [ "$PIDFILE" != "" ]
then
  echo $$ > "$PIDFILE"

  # We created the PIDFILE, so we
  # clean it up.
  trap "rm -f $PIDFILE" EXIT
fi

# Read parset
[ -f "$PARSET" -a -r "$PARSET" ] || error "Cannot read parset: $PARSET"

OBSID=`getkey Observation.ObsID`
echo "Observation ID: $OBSID"

# ******************************
# Preprocess: augment parset
# ******************************

if [ "$AUGMENT_PARSET" -eq "1" ]
then
  AUGMENTED_PARSET=$LOFARROOT/var/run/rtcp-$OBSID.parset

  # Add static keys
  cat $LOFARROOT/etc/parset-additions.d/default/*.parset \
      $HOME/.cobalt/default/*.parset \
      $PARSET \
      $LOFARROOT/etc/parset-additions.d/override/*.parset \
      $HOME/.cobalt/override/*.parset \
      > $AUGMENTED_PARSET || error "Could not create parset $AUGMENTED_PARSET"

  # If we force localhost, we need to remove the node list, or the first one will be used
  if [ "$FORCE_LOCALHOST" -eq "1" ]
  then
    echo "Cobalt.Nodes = []" >> $AUGMENTED_PARSET
  fi

  # Use the new one from now on
  PARSET="$AUGMENTED_PARSET"
fi

# ******************************
# Run the observation
# ******************************

# Determine node list to run on
if [ "$FORCE_LOCALHOST" -eq "1" ]
then
  HOSTS=localhost
else
  HOSTS=`mpi_node_list -n "$PARSET"`
fi

echo "Hosts: $HOSTS"

# Copy parset to all hosts
for h in `echo $HOSTS | tr ',' ' '`
do
  # Ignore empty hostnames
  [ -z "$h" ] && continue;

  # Ignore hostnames that point to us
  [ "$h" == "localhost" ] && continue;
  [ "$h" == "`hostname`" ] && continue;

  # Ignore hosts that already have the parset
  # (for example, through NFS).
  timeout -k1 5s ssh -qn $h [ -e $PARSET ] && continue;

  # Copy parset to remote node
  echo "Copying parset to $h:$PARSET"
  timeout -k1 30s scp -Bq $PARSET $h:$PARSET || error "Could not copy parset to $h"
done

# Run in the background to allow signals to propagate
#
# -x LOFARROOT    Propagate $LOFARROOT for rtcp to find GPU kernels, config files, etc.
# -H              The host list to run on, derived earlier.
mpirun.sh -x LOFARROOT="$LOFARROOT" \
          -H "$HOSTS" \
          $MPIRUN_PARAMS \
          `which rtcp` $RTCP_PARAMS "$PARSET" &
PID=$!

# Propagate SIGTERM
trap "echo runObservation.sh: killing $PID; kill $PID" SIGTERM SIGINT SIGQUIT SIGHUP

# Wait for $COMMAND to finish. We use 'wait' because it will exit immediately if it
# receives a signal.
#
# Return code:
#   signal:    >128
#   no signal: return code of mpirun.sh
wait $PID
OBSRESULT=$?

echo "Result code of observation: $OBSRESULT"

# ******************************
# Post-process the observation
# ******************************
#
# Note: don't propagate errors here as observation failure,
#       because that would be too harsh and also makes testing
#       harder.

if [ "$ONLINECONTROL_FEEDBACK" -eq "1" ]
then
  if [ $OBSRESULT -eq 0 ]
  then
    # ***** Observation ran successfully
    ONLINECONTROL_USER=`getkey Cobalt.Feedback.userName $USER`
    ONLINECONTROL_HOST=`getkey Cobalt.Feedback.host`

    # Copy LTA feedback file to ccu001
    FEEDBACK_DEST=$ONLINECONTROL_USER@$ONLINECONTROL_HOST:`getkey Cobalt.Feedback.remotePath`
    FEEDBACK_FILE=$LOFARROOT/var/run/Observation${OBSID}_feedback
    echo "Copying feedback to $FEEDBACK_DEST"
    timeout -k2 30s scp $FEEDBACK_FILE $FEEDBACK_DEST
    FEEDBACK_RESULT=$?
    if [ $FEEDBACK_RESULT -ne 0 ]
    then
      echo "Failed to copy file $FEEDBACK_FILE to $FEEDBACK_DEST (status: $FEEDBACK_RESULT)"
      OBSRESULT=$FEEDBACK_RESULT
    fi
  fi

  # Communicate result back to OnlineControl
  ONLINECONTROL_RESULT_PORT=$((21000 + $OBSID % 1000))

  if [ $OBSRESULT -eq 0 ]
  then
    # Signal success to OnlineControl
    echo "Signalling success to $ONLINECONTROL_HOST"
    echo -n "FINISHED" > /dev/tcp/$ONLINECONTROL_HOST/$ONLINECONTROL_RESULT_PORT
  else
    # ***** Observation or sending feedback failed for some reason
    # Signal failure to OnlineControl
    echo "Signalling failure to $ONLINECONTROL_HOST"
    echo -n "ABORT" > /dev/tcp/$ONLINECONTROL_HOST/$ONLINECONTROL_RESULT_PORT
  fi
else
  echo "Not communicating back to OnlineControl"
fi

# Our exit code is that of the observation
exit $OBSRESULT

