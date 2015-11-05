#!/bin/bash
#
# To record raw data:
#
# 1. Define an observation in OTDB.
# 2. Disable Cobalt, by either:
#      a. removing OnlineControl from the VIC tree
#      b. moving/disabling cbm001:/opt/lofar/bin/startBGL.sh
# 3. Adjust the following parameters in this script:
#      a. NAME, used as an ID in the file names that will be created
#      b. FILTERPARAMS (# subbands/board, start time, stop time)
#      c. STATIONS, FIELD, and BOARDS
#      d. OUTPUT (to route to Cobalt disk (< ~2min obs) or to locus nodes)
#      e. LOCUS_NODES, if routing to locus
# 4. Run this script, which produces:
#       record.sh:     the script to run to record station data
#       replay.sh:     the script to run to replay the recorded station data
#       replay.parset: parset keys needed to replay the recorded station data
# 5. Run record.sh before the obs starts

# Sane default
if [ -z "$LOFARROOT" ]; then
  LOFARROOT=/opt/lofar
fi

# Raw parameters to filterRSP
# -q: Quiet, don't spam progress logs
# -s: Number of subbands to record [SET TO NUMBER OF SUBBANDS]
# -f: Record from this timestamp [SET TO OBS START TIME - 1SEC]
# -t: Record until this timestamp [SET TO OBS END TIME]
FILTERPARAMS="-s 122 -q -f '2014-05-22 13:34:59' -t '2014-05-22 13:40:01'"

# Identifier for the output files [SET TO SOMETHING UNIQUE]
NAME="cs-is-scaling"

# Just a note containing all stations
ALLSTATIONS="
  CS001
  CS002 CS003 CS004 CS005 CS006 CS007
  CS011 CS013 CS017
  CS021 CS024 CS026 CS028
  CS030 CS031 CS032
  CS101 CS103
  CS201
  CS301 CS302
  CS401
  CS501
  RS106
  RS205 RS208 RS210
  RS305 RS306 RS307 RS310
  RS406 RS407 RS409
  RS503 RS508 RS509

  DE601 DE602 DE603 DE604 DE605
  FR606 SE607 UK608
"

# Antennaset to record [SET TO ANTENNA SET]
FIELD=HBA_DUAL

# Which board(s) to record
BOARDS="0"

# Stations to record [SET TO STATION LIST]
STATIONS="
  CS001
  CS002 CS003 CS004 CS005 CS006 CS007
  CS011 CS017
  CS021 CS024 CS026 CS028
  CS030 CS031 CS032
  CS101 CS103
  CS201
  CS301 CS302
  CS401
  CS501
"

# Output to "locus" or to "file"
OUTPUT=locus

# List of locus nodes, and first port number to use (if OUTPUT=locus)
#
# To sort locus nodes on disk space available on /data, use:
#
# [lhn001] cexec -p df /data | grep sda10 | sort -n -k 7 | cut -c 12-14 | tr '\n' ' '
LOCUS_NODES=(055 100 080 065 093 094 058 043 004 022 034 037 059 083 045 062 067 090 010 021 023 033 066 087 009 013 029 056 069 071 079 086 019 039 064 014 035 050 053 088 001 016 026 041 051 054 060 061 077 082 089 005 007 008 068 085 017 018 020 038 049 076 091 002 047 063 098 012 027 072 006 074 024 031 057 075 078 052 070 011 046 048 003 040 081 015 025 042 044 084 095 096 032 073 028 036 097 030 092 099)
LOCUS_FIRST_PORT=12345

# Temporary location for the parset we create
PARSET=record_raw_data.parset

# We'll need a timestamp that won't change during this script
NOW=`date +%FT%T`

# Construct the list of all fields (f.e. CS001HBA0 CS001HBA1 RS106HBA)
FIELDS=""
for s in $STATIONS
do
  case $s in
    CS*)
      case $FIELD in
        LBA*)
          FIELDS="$FIELDS ${s}LBA"
          ;;
        HBA_ZERO*)
          FIELDS="$FIELDS ${s}HBA0"
          ;;
        HBA_ONE*)
          FIELDS="$FIELDS ${s}HBA0"
          ;;
        HBA_DUAL*)
          FIELDS="$FIELDS ${s}HBA0 ${s}HBA1"
          ;;
        HBA_JOINED*)
          FIELDS="$FIELDS ${s}HBA"
          ;;
      esac
      ;;
    *)
      case $FIELD in
        LBA*)
          FIELDS="$FIELDS ${s}LBA"
          ;;
        HBA*)
          FIELDS="$FIELDS ${s}HBA"
          ;;
      esac
      ;;
  esac
done

(
# Create a basic configuration
echo "Observation.Beam[0].subbandList = [0]"
echo "Observation.VirtualInstrument.stationList = [`echo $FIELDS | tr ' ' ','`]"
echo "Observation.rspBoardList = [0]"
echo "Observation.rspSlotList  = [0]"

# Add all input specifications
cat $LOFARROOT/etc/parset-additions.d/default/*.parset
)> $PARSET

LOCUSIDX=0

# Reset replay scripts
rm -f record.sh replay.sh replay.parset

# Start recording all fields
for s in $FIELDS
do
  # Determine the host for this field
  HOST=`$LOFARROOT/bin/station_stream -S $s -h $PARSET 2>/dev/null`;

  # List of input streams for the observation that is going to process this data
  OBS_INSTREAMS=""

  for b in $BOARDS
  do
    # The input stream
    INSTREAM=`$LOFARROOT/bin/station_stream -S $s -B $b -s $PARSET 2>/dev/null`;

    # The interface on which the stream is received
    IFACE=`echo $INSTREAM | cut -d: -f 2`

    # The socket to bind to on the node
    CPU=`$LOFARROOT/bin/station_stream -S $s -B $b -c $PARSET 2>/dev/null`;

    # The output stream
    case "$OUTPUT" in
      file)
        OUTSTREAM="file:/localhome/mol/raw-$NAME-$NOW-$s-$b.udp"
        OBS_INSTREAM="$OUTSTREAM"
        ;;
      locus)
        NR=${#LOCUS_NODES[@]}
        DESTNODENR=`echo "$LOCUSIDX % $NR" | bc`
        DESTNODE="locus${LOCUS_NODES[$DESTNODENR]}"
        DESTPORT=$((LOCUS_FIRST_PORT + LOCUSIDX))
        LOCUSIDX=$((LOCUSIDX + 1))

        OUTSTREAM="tcp:$DESTNODE:$DESTPORT"
        FILESTREAM="file:/data/raw-$NAME-$NOW-$s-$b.udp"

        echo "# stream $s board $b [$OUTSTREAM -> $FILESTREAM]" >> record.sh
        echo ssh $DESTNODE \"/globalhome/romein/bin.x86_64/udp-copy tcp:0:$DESTPORT $FILESTREAM\" "&" >> record.sh

        OBS_INSTREAM="tcp:$IFACE:$DESTPORT"
        echo "ssh $DESTNODE \"/globalhome/romein/bin.x86_64/udp-copy $FILESTREAM $INSTREAM \"" "&" >> replay.sh
        ;;
    esac

    if [ -z "$OBS_INSTREAMS" ]; then
      OBS_INSTREAMS="$OBS_INSTREAM"
    else
      OBS_INSTREAMS="$OBS_INSTREAMS,$OBS_INSTREAM"
    fi

    # The command to execute to record this field
    echo "# stream $s board $b [$INSTREAM -> $OUTSTREAM]" >> record.sh
    echo ssh $HOST \"nice -n -20 numactl --cpunodebind=$CPU --membind=$CPU $LOFARROOT/filterRSP -i $INSTREAM -o $OUTSTREAM "$FILTERPARAMS" \" "&" >> record.sh
  done

  echo "PIC.Core.$s.RSP.ports = [$OBS_INSTREAMS]" >> replay.parset
done

# At the end of all ssh commands, wait for all of them to finish
echo wait >> record.sh
echo wait >> replay.sh
