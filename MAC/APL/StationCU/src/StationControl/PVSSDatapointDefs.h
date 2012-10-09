// This file was generated by create_db_files v2.0 on Thu Jan  5 11:01:56 UTC 2012

#ifndef LOFAR_DEPLOYMENT_PVSSDATAPOINTS_H
#define LOFAR_DEPLOYMENT_PVSSDATAPOINTS_H
// process
#define	PN_FSM_PROCESSID	"process.processID"
#define	PN_FSM_START_TIME	"process.startTime"
#define	PN_FSM_STOP_TIME	"process.stopTime"
#define	PN_FSM_LOG_MSG	"process.logMsg"
#define	PN_FSM_ERROR	"process.error"
#define	PN_FSM_CURRENT_ACTION	"process.currentAction"
// object
#define	PN_OBJ_STATE	"object.state"
#define	PN_OBJ_CHILD_STATE	"object.childState"
#define	PN_OBJ_MESSAGE	"object.message"
#define	PN_OBJ_LEAF	"object.leaf"

// Station
#define PSN_STATION	"LOFAR_PIC_@ring@_@station@"
#define PST_STATION	"Station"
#define PN_STS_POWER48_ON	"power48On"
#define PN_STS_POWER220_ON	"power220On"

// Station
#define PSN_STATION	"LOFAR_PermSW_@ring@_@station@"
#define PST_STATION	"Station"
#define PN_STS_POWER48_ON	"power48On"
#define PN_STS_POWER220_ON	"power220On"

// MACScheduler
#define PSN_MAC_SCHEDULER	"LOFAR_PermSW_MACScheduler"
#define PST_MAC_SCHEDULER	"MACScheduler"
#define PN_MS_PLANNED_OBSERVATIONS	"plannedObservations"
#define PN_MS_ACTIVE_OBSERVATIONS	"activeObservations"
#define PN_MS_FINISHED_OBSERVATIONS	"finishedObservations"
#define PN_MS_OTDB_CONNECTED	"OTDB.connected"
#define PN_MS_OTDB_LAST_POLL	"OTDB.lastPoll"
#define PN_MS_OTDB_POLLINTERVAL	"OTDB.pollinterval"

// Observation
#define PSN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_OBSERVATION	"Observation"
#define	PN_OBS_CLAIM_CLAIM_DATE	"claim.claimDate"
#define	PN_OBS_CLAIM_NAME	"claim.name"
#define PN_OBS_CLAIM_PERIOD	"claimPeriod"
#define PN_OBS_PREPARE_PERIOD	"preparePeriod"
#define PN_OBS_START_TIME	"startTime"
#define PN_OBS_STOP_TIME	"stopTime"
#define PN_OBS_BAND_FILTER	"bandFilter"
#define PN_OBS_NYQUISTZONE	"nyquistzone"
#define PN_OBS_ANTENNA_ARRAY	"antennaArray"
#define PN_OBS_RECEIVER_LIST	"receiverList"
#define PN_OBS_SAMPLE_CLOCK	"sampleClock"
#define PN_OBS_RUN_STATE	"runState"
#define PN_OBS_MEASUREMENT_SET	"measurementSet"
#define PN_OBS_STATION_LIST	"stationList"
#define PN_OBS_INPUT_NODE_LIST	"inputNodeList"
#define PN_OBS_BGL_NODE_LIST	"BGLNodeList"
#define PN_OBS_STORAGE_NODE_LIST	"storageNodeList"
#define PN_OBS_BEAMS_ANGLE1	"Beams.angle1"
#define PN_OBS_BEAMS_ANGLE2	"Beams.angle2"
#define PN_OBS_BEAMS_DIRECTION_TYPE	"Beams.directionType"
#define PN_OBS_BEAMS_BEAMLET_LIST	"Beams.beamletList"
#define PN_OBS_BEAMS_SUBBAND_LIST	"Beams.subbandList"

// ObservationControl
#define PSN_OBSERVATION_CONTROL	"LOFAR_ObsSW_@observation@_ObservationControl"
#define PST_OBSERVATION_CONTROL	"ObservationControl"

// InputBuffer
#define PSN_INPUT_BUFFER	"LOFAR_PermSW_@psionode@_InputBuffer"
#define PST_INPUT_BUFFER	"InputBuffer"
#define PN_IPB_STATION_NAME	"stationName"
#define PN_IPB_OBSERVATION_NAME	"observationName"
#define PN_IPB_STREAM0_BLOCKS_IN	"stream0.blocksIn"
#define PN_IPB_STREAM0_PERC_BAD	"stream0.percBad"
#define PN_IPB_STREAM0_REJECTED	"stream0.rejected"
#define PN_IPB_STREAM1_BLOCKS_IN	"stream1.blocksIn"
#define PN_IPB_STREAM1_PERC_BAD	"stream1.percBad"
#define PN_IPB_STREAM1_REJECTED	"stream1.rejected"
#define PN_IPB_STREAM2_BLOCKS_IN	"stream2.blocksIn"
#define PN_IPB_STREAM2_PERC_BAD	"stream2.percBad"
#define PN_IPB_STREAM2_REJECTED	"stream2.rejected"
#define PN_IPB_STREAM3_BLOCKS_IN	"stream3.blocksIn"
#define PN_IPB_STREAM3_PERC_BAD	"stream3.percBad"
#define PN_IPB_STREAM3_REJECTED	"stream3.rejected"
#define PN_IPB_LATE	"late"
#define PN_IPB_IO_TIME	"IOTime"

// Adder
#define PSN_ADDER	"LOFAR_ObsSW_@osionode@_@adder@"
#define PST_ADDER	"Adder"
#define PN_ADD_DROPPING	"dropping"
#define PN_ADD_DROPPED	"dropped"
#define PN_ADD_DATA_PRODUCT_TYPE	"dataProductType"
#define PN_ADD_FILE_NAME	"fileName"
#define PN_ADD_LOCUS_NODE	"locusNode"
#define PN_ADD_DIRECTORY	"directory"
#define PN_ADD_OBSERVATION_NAME	"observationName"

// Writer
#define PSN_WRITER	"LOFAR_ObsSW_@oslocusnode@_@writer@"
#define PST_WRITER	"Writer"
#define PN_WTR_TIME	"time"
#define PN_WTR_COUNT	"count"
#define PN_WTR_DROPPED	"dropped"
#define PN_WTR_FILE_NAME	"fileName"
#define PN_WTR_DATA_RATE	"dataRate"
#define PN_WTR_DATA_PRODUKT_TYPE	"dataProduktType"
#define PN_WTR_OBSERVATION_NAME	"observationName"

// IONode
#define PSN_IO_NODE	"LOFAR_PIC_BGP_@midplane@_@ionode@"
#define PST_IO_NODE	"IONode"
#define PN_ION_STATION0	"station0"
#define PN_ION_IP0	"IP0"
#define PN_ION_MAC0	"MAC0"
#define PN_ION_STATION1	"station1"
#define PN_ION_IP1	"IP1"
#define PN_ION_MAC1	"MAC1"
#define PN_ION_USE2ND_STATION	"use2ndStation"
#define PN_ION_USED_STATION	"usedStation"
#define PN_ION_USEDIP	"usedIP"
#define PN_ION_USEDMAC	"usedMAC"

// LocusNode
#define PSN_LOCUS_NODE	"LOFAR_PIC_@osrack@_@locusnode@"
#define PST_LOCUS_NODE	"LocusNode"
#define PN_LCN_FREE	"free"
#define PN_LCN_TOTAL	"total"
#define PN_LCN_CLAIMED	"claimed"

// CEPHardwareMonitor
#define PSN_CEP_HARDWARE_MONITOR	"LOFAR_PermSW_HardwareMonitor"
#define PST_CEP_HARDWARE_MONITOR	"CEPHardwareMonitor"
#define PN_CHM_BGP_CONNECTED	"BGP.connected"
#define PN_CHM_CLUSTER_CONNECTED	"Cluster.connected"

// Cabinet
#define PSN_CABINET	"LOFAR_PIC_@cabinet@"
#define PST_CABINET	"Cabinet"
#define PN_CAB_FRONT_DOOR_OPEN	"frontDoorOpen"
#define PN_CAB_FRONT_FAN_INNER	"frontFanInner"
#define PN_CAB_FRONT_FAN_OUTER	"frontFanOuter"
#define PN_CAB_FRONT_AIRFLOW	"frontAirflow"
#define PN_CAB_BACK_DOOR_OPEN	"backDoorOpen"
#define PN_CAB_BACK_FAN_INNER	"backFanInner"
#define PN_CAB_BACK_FAN_OUTER	"backFanOuter"
#define PN_CAB_BACK_AIRFLOW	"backAirflow"
#define PN_CAB_TEMP_ALARM	"tempAlarm"
#define PN_CAB_HUMIDITY_ALARM	"humidityAlarm"
#define PN_CAB_TEMPERATURE	"temperature"
#define PN_CAB_TEMP_MIN	"tempMin"
#define PN_CAB_TEMP_MAX	"tempMax"
#define PN_CAB_TEMP_MAX_MAX	"tempMaxMax"
#define PN_CAB_HUMIDITY	"humidity"
#define PN_CAB_HUMIDITY_MAX	"humidityMax"
#define PN_CAB_HUMIDITY_MAX_MAX	"humidityMaxMax"
#define PN_CAB_CONTROL_MODE	"controlMode"
#define PN_CAB_TEMPERATURE_SENSOR	"temperatureSensor"
#define PN_CAB_HUMIDITY_CONTROL	"humidityControl"
#define PN_CAB_DOOR_CONTROL	"doorControl"

// SubRack
#define PSN_SUB_RACK	"LOFAR_PIC_@cabinet@_@subrack@"
#define PST_SUB_RACK	"SubRack"
#define	PN_SRCK_SPU_STATUS_STATE	"SPU.status.state"
#define	PN_SRCK_SPU_STATUS_CHILD_STATE	"SPU.status.childState"
#define	PN_SRCK_SPU_STATUS_MESSAGE	"SPU.status.message"
#define	PN_SRCK_SPU_STATUS_LEAF	"SPU.status.leaf"
#define PN_SRCK_SPU__VHBA	"SPU.Vhba"
#define PN_SRCK_SPU__VLBA	"SPU.Vlba"
#define PN_SRCK_SPU__VDIG	"SPU.Vdig"
#define PN_SRCK_SPU_TEMPERATURE	"SPU.temperature"
#define	PN_SRCK_CLOCK_BOARD_STATUS_STATE	"clockBoard.status.state"
#define	PN_SRCK_CLOCK_BOARD_STATUS_CHILD_STATE	"clockBoard.status.childState"
#define	PN_SRCK_CLOCK_BOARD_STATUS_MESSAGE	"clockBoard.status.message"
#define	PN_SRCK_CLOCK_BOARD_STATUS_LEAF	"clockBoard.status.leaf"
#define PN_SRCK_CLOCK_BOARD__VFSP	"clockBoard.Vfsp"
#define PN_SRCK_CLOCK_BOARD__VCLOCK	"clockBoard.Vclock"
#define PN_SRCK_CLOCK_BOARD_VERSION	"clockBoard.version"
#define PN_SRCK_CLOCK_BOARD_FREQ	"clockBoard.freq"
#define PN_SRCK_CLOCK_BOARD_LOCK160	"clockBoard.lock160"
#define PN_SRCK_CLOCK_BOARD_LOCK200	"clockBoard.lock200"
#define PN_SRCK_CLOCK_BOARD_TEMPERATURE	"clockBoard.temperature"

// RSPBoard
#define PSN_RSP_BOARD	"LOFAR_PIC_@cabinet@_@subrack@_@RSPBoard@"
#define PST_RSP_BOARD	"RSPBoard"
#define PN_RSP_VOLTAGE12	"voltage12"
#define PN_RSP_VOLTAGE25	"voltage25"
#define PN_RSP_VOLTAGE33	"voltage33"
#define PN_RSP_VERSION	"version"
#define PN_RSP_ALERT	"alert"
#define PN_RSP_SPLITTER_ON	"splitterOn"
#define	PN_RSP__ETHERNET_STATUS_STATE	"Ethernet.status.state"
#define	PN_RSP__ETHERNET_STATUS_CHILD_STATE	"Ethernet.status.childState"
#define	PN_RSP__ETHERNET_STATUS_MESSAGE	"Ethernet.status.message"
#define	PN_RSP__ETHERNET_STATUS_LEAF	"Ethernet.status.leaf"
#define PN_RSP_ETHERNET_PACKETS_RECEIVED	"Ethernet.packetsReceived"
#define PN_RSP_ETHERNET_PACKETS_ERROR	"Ethernet.packetsError"
#define PN_RSP_ETHERNET_LAST_ERROR	"Ethernet.lastError"
#define PN_RSP_MEP_SEQNR	"MEP.seqnr"
#define PN_RSP_MEP_ERROR	"MEP.error"
#define	PN_RSP_BP_STATUS_STATE	"BP.status.state"
#define	PN_RSP_BP_STATUS_CHILD_STATE	"BP.status.childState"
#define	PN_RSP_BP_STATUS_MESSAGE	"BP.status.message"
#define	PN_RSP_BP_STATUS_LEAF	"BP.status.leaf"
#define PN_RSP_BP_TEMPERATURE	"BP.temperature"
#define PN_RSP_BP_VERSION	"BP.version"
#define	PN_RSP_AP0_STATUS_STATE	"AP0.status.state"
#define	PN_RSP_AP0_STATUS_CHILD_STATE	"AP0.status.childState"
#define	PN_RSP_AP0_STATUS_MESSAGE	"AP0.status.message"
#define	PN_RSP_AP0_STATUS_LEAF	"AP0.status.leaf"
#define PN_RSP_AP0_TEMPERATURE	"AP0.temperature"
#define PN_RSP_AP0_VERSION	"AP0.version"
#define PN_RSP_AP0_SYNC_SAMPLE_COUNT	"AP0.SYNC.sampleCount"
#define PN_RSP_AP0_SYNC_SYNC_COUNT	"AP0.SYNC.syncCount"
#define PN_RSP_AP0_SYNC_ERROR_COUNT	"AP0.SYNC.errorCount"
#define	PN_RSP_AP1_STATUS_STATE	"AP1.status.state"
#define	PN_RSP_AP1_STATUS_CHILD_STATE	"AP1.status.childState"
#define	PN_RSP_AP1_STATUS_MESSAGE	"AP1.status.message"
#define	PN_RSP_AP1_STATUS_LEAF	"AP1.status.leaf"
#define PN_RSP_AP1_TEMPERATURE	"AP1.temperature"
#define PN_RSP_AP1_VERSION	"AP1.version"
#define PN_RSP_AP1_SYNC_SAMPLE_COUNT	"AP1.SYNC.sampleCount"
#define PN_RSP_AP1_SYNC_SYNC_COUNT	"AP1.SYNC.syncCount"
#define PN_RSP_AP1_SYNC_ERROR_COUNT	"AP1.SYNC.errorCount"
#define	PN_RSP_AP2_STATUS_STATE	"AP2.status.state"
#define	PN_RSP_AP2_STATUS_CHILD_STATE	"AP2.status.childState"
#define	PN_RSP_AP2_STATUS_MESSAGE	"AP2.status.message"
#define	PN_RSP_AP2_STATUS_LEAF	"AP2.status.leaf"
#define PN_RSP_AP2_TEMPERATURE	"AP2.temperature"
#define PN_RSP_AP2_VERSION	"AP2.version"
#define PN_RSP_AP2_SYNC_SAMPLE_COUNT	"AP2.SYNC.sampleCount"
#define PN_RSP_AP2_SYNC_SYNC_COUNT	"AP2.SYNC.syncCount"
#define PN_RSP_AP2_SYNC_ERROR_COUNT	"AP2.SYNC.errorCount"
#define	PN_RSP_AP3_STATUS_STATE	"AP3.status.state"
#define	PN_RSP_AP3_STATUS_CHILD_STATE	"AP3.status.childState"
#define	PN_RSP_AP3_STATUS_MESSAGE	"AP3.status.message"
#define	PN_RSP_AP3_STATUS_LEAF	"AP3.status.leaf"
#define PN_RSP_AP3_TEMPERATURE	"AP3.temperature"
#define PN_RSP_AP3_VERSION	"AP3.version"
#define PN_RSP_AP3_SYNC_SAMPLE_COUNT	"AP3.SYNC.sampleCount"
#define PN_RSP_AP3_SYNC_SYNC_COUNT	"AP3.SYNC.syncCount"
#define PN_RSP_AP3_SYNC_ERROR_COUNT	"AP3.SYNC.errorCount"

// RCU
#define PSN_RCU	"LOFAR_PIC_@cabinet@_@subrack@_@RSPBoard@_@rcu@"
#define PST_RCU	"RCU"
#define PN_RCU_DELAY	"Delay"
#define PN_RCU_INPUT_ENABLE	"InputEnable"
#define PN_RCU_LBL_ENABLE	"LBLEnable"
#define PN_RCU_LBH_ENABLE	"LBHEnable"
#define PN_RCU_HBA_ENABLE	"HBAEnable"
#define PN_RCU_BAND_SEL_LBA_HBA	"bandSelLbaHba"
#define PN_RCU_HBA_FILTER_SEL	"HBAFilterSel"
#define PN_RCU_VL_ENABLE	"VlEnable"
#define PN_RCU_VH_ENABLE	"VhEnable"
#define PN_RCU_VDD_VCC_ENABLE	"VddVccEnable"
#define PN_RCU_BAND_SEL_LBL_LBH	"bandSelLblLbh"
#define PN_RCU_LBA_FILTER_SEL	"LBAFilterSel"
#define PN_RCU_ATTENUATION	"Attenuation"
#define PN_RCU_NOF_OVERFLOW	"nofOverflow"
#define PN_RCU_ADC_STATISTICS_OVERFLOW	"ADCStatistics.overflow"
#define PN_RCU_TBB_ERROR	"TBB.error"
#define PN_RCU_TBB_MODE	"TBB.mode"
#define PN_RCU_TBB_START_ADDR	"TBB.startAddr"
#define PN_RCU_TBB_BUF_SIZE	"TBB.bufSize"
#define PN_RCU_TRIGGER_STARTLEVEL	"Trigger.startlevel"
#define PN_RCU_TRIGGER_BASELEVEL	"Trigger.baselevel"
#define PN_RCU_TRIGGER_STOPLEVEL	"Trigger.stoplevel"
#define PN_RCU_TRIGGER_FILTER	"Trigger.filter"
#define PN_RCU_TRIGGER_WINDOW	"Trigger.window"
#define PN_RCU_TRIGGER_OPERATING_MODE	"Trigger.operatingMode"
#define PN_RCU_TRIGGER_TRIGGER_MODE	"Trigger.triggerMode"
#define PN_RCU_TRIGGER_FILTER0_COEFF0	"Trigger.filter0.coeff0"
#define PN_RCU_TRIGGER_FILTER0_COEFF1	"Trigger.filter0.coeff1"
#define PN_RCU_TRIGGER_FILTER0_COEFF2	"Trigger.filter0.coeff2"
#define PN_RCU_TRIGGER_FILTER0_COEFF3	"Trigger.filter0.coeff3"
#define PN_RCU_TRIGGER_FILTER1_COEFF0	"Trigger.filter1.coeff0"
#define PN_RCU_TRIGGER_FILTER1_COEFF1	"Trigger.filter1.coeff1"
#define PN_RCU_TRIGGER_FILTER1_COEFF2	"Trigger.filter1.coeff2"
#define PN_RCU_TRIGGER_FILTER1_COEFF3	"Trigger.filter1.coeff3"

// TBBoard
#define PSN_TB_BOARD	"LOFAR_PIC_@cabinet@_@subrack@_@TBBoard@"
#define PST_TB_BOARD	"TBBoard"
#define PN_TBB_BOARDID	"boardID"
#define PN_TBB_RAM_SIZE	"RAMSize"
#define PN_TBB_SW_VERSION	"SWVersion"
#define PN_TBB_BOARD_VERSION	"boardVersion"
#define PN_TBB_TP_VERSION	"TPVersion"
#define PN_TBB_MP0_VERSION	"MP0Version"
#define PN_TBB_MP1_VERSION	"MP1Version"
#define PN_TBB_MP2_VERSION	"MP2Version"
#define PN_TBB_MP3_VERSION	"MP3Version"
#define PN_TBB_VOLTAGE12	"voltage12"
#define PN_TBB_VOLTAGE25	"voltage25"
#define PN_TBB_VOLTAGE33	"voltage33"
#define PN_TBB_TEMPPCB	"tempPCB"
#define PN_TBB_TEMPTP	"tempTP"
#define PN_TBB_TEMPMP0	"tempMP0"
#define PN_TBB_TEMPMP1	"tempMP1"
#define PN_TBB_TEMPMP2	"tempMP2"
#define PN_TBB_TEMPMP3	"tempMP3"
#define PN_TBB_IMAGE_INFO_VERSION	"imageInfo.version"
#define PN_TBB_IMAGE_INFO_WRITE_DATE	"imageInfo.writeDate"
#define PN_TBB_IMAGE_INFO_TP_FILE	"imageInfo.TPFile"
#define PN_TBB_IMAGE_INFO_MP_FILE	"imageInfo.MPFile"

// LBAAntenna
#define PSN_LBA_ANTENNA	"LOFAR_PIC_@lbaantenna@"
#define PST_LBA_ANTENNA	"LBAAntenna"
#define	PN_LBA_COMMON_RCUX	"common.RCUX"
#define	PN_LBA_COMMON_RCUY	"common.RCUY"
#define	PN_LBA_COMMON_DELTAX	"common.deltaX"
#define	PN_LBA_COMMON_DELTAY	"common.deltaY"
#define	PN_LBA_COMMON_DELTAZ	"common.deltaZ"

// HBAAntenna
#define PSN_HBA_ANTENNA	"LOFAR_PIC_@hbaantenna@"
#define PST_HBA_ANTENNA	"HBAAntenna"
#define	PN_HBA_COMMON_RCUX	"common.RCUX"
#define	PN_HBA_COMMON_RCUY	"common.RCUY"
#define	PN_HBA_COMMON_DELTAX	"common.deltaX"
#define	PN_HBA_COMMON_DELTAY	"common.deltaY"
#define	PN_HBA_COMMON_DELTAZ	"common.deltaZ"
#define	PN_HBA_ELEMENT00_STATUS_STATE	"element00.status.state"
#define	PN_HBA_ELEMENT00_STATUS_CHILD_STATE	"element00.status.childState"
#define	PN_HBA_ELEMENT00_STATUS_MESSAGE	"element00.status.message"
#define	PN_HBA_ELEMENT00_STATUS_LEAF	"element00.status.leaf"
#define	PN_HBA_ELEMENT01_STATUS_STATE	"element01.status.state"
#define	PN_HBA_ELEMENT01_STATUS_CHILD_STATE	"element01.status.childState"
#define	PN_HBA_ELEMENT01_STATUS_MESSAGE	"element01.status.message"
#define	PN_HBA_ELEMENT01_STATUS_LEAF	"element01.status.leaf"
#define	PN_HBA_ELEMENT02_STATUS_STATE	"element02.status.state"
#define	PN_HBA_ELEMENT02_STATUS_CHILD_STATE	"element02.status.childState"
#define	PN_HBA_ELEMENT02_STATUS_MESSAGE	"element02.status.message"
#define	PN_HBA_ELEMENT02_STATUS_LEAF	"element02.status.leaf"
#define	PN_HBA_ELEMENT03_STATUS_STATE	"element03.status.state"
#define	PN_HBA_ELEMENT03_STATUS_CHILD_STATE	"element03.status.childState"
#define	PN_HBA_ELEMENT03_STATUS_MESSAGE	"element03.status.message"
#define	PN_HBA_ELEMENT03_STATUS_LEAF	"element03.status.leaf"
#define	PN_HBA_ELEMENT04_STATUS_STATE	"element04.status.state"
#define	PN_HBA_ELEMENT04_STATUS_CHILD_STATE	"element04.status.childState"
#define	PN_HBA_ELEMENT04_STATUS_MESSAGE	"element04.status.message"
#define	PN_HBA_ELEMENT04_STATUS_LEAF	"element04.status.leaf"
#define	PN_HBA_ELEMENT05_STATUS_STATE	"element05.status.state"
#define	PN_HBA_ELEMENT05_STATUS_CHILD_STATE	"element05.status.childState"
#define	PN_HBA_ELEMENT05_STATUS_MESSAGE	"element05.status.message"
#define	PN_HBA_ELEMENT05_STATUS_LEAF	"element05.status.leaf"
#define	PN_HBA_ELEMENT06_STATUS_STATE	"element06.status.state"
#define	PN_HBA_ELEMENT06_STATUS_CHILD_STATE	"element06.status.childState"
#define	PN_HBA_ELEMENT06_STATUS_MESSAGE	"element06.status.message"
#define	PN_HBA_ELEMENT06_STATUS_LEAF	"element06.status.leaf"
#define	PN_HBA_ELEMENT07_STATUS_STATE	"element07.status.state"
#define	PN_HBA_ELEMENT07_STATUS_CHILD_STATE	"element07.status.childState"
#define	PN_HBA_ELEMENT07_STATUS_MESSAGE	"element07.status.message"
#define	PN_HBA_ELEMENT07_STATUS_LEAF	"element07.status.leaf"
#define	PN_HBA_ELEMENT08_STATUS_STATE	"element08.status.state"
#define	PN_HBA_ELEMENT08_STATUS_CHILD_STATE	"element08.status.childState"
#define	PN_HBA_ELEMENT08_STATUS_MESSAGE	"element08.status.message"
#define	PN_HBA_ELEMENT08_STATUS_LEAF	"element08.status.leaf"
#define	PN_HBA_ELEMENT09_STATUS_STATE	"element09.status.state"
#define	PN_HBA_ELEMENT09_STATUS_CHILD_STATE	"element09.status.childState"
#define	PN_HBA_ELEMENT09_STATUS_MESSAGE	"element09.status.message"
#define	PN_HBA_ELEMENT09_STATUS_LEAF	"element09.status.leaf"
#define	PN_HBA_ELEMENT10_STATUS_STATE	"element10.status.state"
#define	PN_HBA_ELEMENT10_STATUS_CHILD_STATE	"element10.status.childState"
#define	PN_HBA_ELEMENT10_STATUS_MESSAGE	"element10.status.message"
#define	PN_HBA_ELEMENT10_STATUS_LEAF	"element10.status.leaf"
#define	PN_HBA_ELEMENT11_STATUS_STATE	"element11.status.state"
#define	PN_HBA_ELEMENT11_STATUS_CHILD_STATE	"element11.status.childState"
#define	PN_HBA_ELEMENT11_STATUS_MESSAGE	"element11.status.message"
#define	PN_HBA_ELEMENT11_STATUS_LEAF	"element11.status.leaf"
#define	PN_HBA_ELEMENT12_STATUS_STATE	"element12.status.state"
#define	PN_HBA_ELEMENT12_STATUS_CHILD_STATE	"element12.status.childState"
#define	PN_HBA_ELEMENT12_STATUS_MESSAGE	"element12.status.message"
#define	PN_HBA_ELEMENT12_STATUS_LEAF	"element12.status.leaf"
#define	PN_HBA_ELEMENT13_STATUS_STATE	"element13.status.state"
#define	PN_HBA_ELEMENT13_STATUS_CHILD_STATE	"element13.status.childState"
#define	PN_HBA_ELEMENT13_STATUS_MESSAGE	"element13.status.message"
#define	PN_HBA_ELEMENT13_STATUS_LEAF	"element13.status.leaf"
#define	PN_HBA_ELEMENT14_STATUS_STATE	"element14.status.state"
#define	PN_HBA_ELEMENT14_STATUS_CHILD_STATE	"element14.status.childState"
#define	PN_HBA_ELEMENT14_STATUS_MESSAGE	"element14.status.message"
#define	PN_HBA_ELEMENT14_STATUS_LEAF	"element14.status.leaf"
#define	PN_HBA_ELEMENT15_STATUS_STATE	"element15.status.state"
#define	PN_HBA_ELEMENT15_STATUS_CHILD_STATE	"element15.status.childState"
#define	PN_HBA_ELEMENT15_STATUS_MESSAGE	"element15.status.message"
#define	PN_HBA_ELEMENT15_STATUS_LEAF	"element15.status.leaf"

// LogProcessor
#define PSN_LOG_PROCESSOR	"LOFAR_PermSW_Daemons_LogProcessor"
#define PST_LOG_PROCESSOR	"LogProcessor"

// SASGateway
#define PSN_SAS_GATEWAY	"LOFAR_PermSW_Daemons_SASGateway"
#define PST_SAS_GATEWAY	"SASGateway"

// HardwareMonitor
#define PSN_HARDWARE_MONITOR	"LOFAR_PermSW_HardwareMonitor"
#define PST_HARDWARE_MONITOR	"HardwareMonitor"
#define PN_HWM_RSP_CONNECTED	"RSP.connected"
#define PN_HWM_TBB_CONNECTED	"TBB.connected"
#define PN_HWM_EC_CONNECTED	"EC.connected"

// SoftwareMonitor
#define PSN_SOFTWARE_MONITOR	"LOFAR_PermSW_SoftwareMonitor"
#define PST_SOFTWARE_MONITOR	"SoftwareMonitor"

// SHMInfoServer
#define PSN_SHM_INFO_SERVER	"LOFAR_PermSW_SHMInfoServer"
#define PST_SHM_INFO_SERVER	"SHMInfoServer"

// StationControl
#define PSN_STATION_CONTROL	"LOFAR_PermSW_StationControl"
#define PST_STATION_CONTROL	"StationControl"
#define PN_SC_ACTIVE_OBSERVATIONS	"activeObservations"

// ClockControl
#define PSN_CLOCK_CONTROL	"LOFAR_PermSW_ClockControl"
#define PST_CLOCK_CONTROL	"ClockControl"
#define PN_CLC_CONNECTED	"connected"
#define PN_CLC_REQUESTED_CLOCK	"requestedClock"
#define PN_CLC_ACTUAL_CLOCK	"actualClock"
#define PN_CLC_REQUESTED_BITMODE	"requestedBitmode"
#define PN_CLC_ACTUAL_BITMODE	"actualBitmode"

// StnObservation
#define PSN_STN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_STN_OBSERVATION	"StnObservation"
#define PN_OBS_NAME	"name"
#define	PN_OBS_CLAIM_CLAIM_DATE	"claim.claimDate"
#define	PN_OBS_CLAIM_NAME	"claim.name"
#define PN_OBS_RECEIVER_BITMAP	"receiverBitmap"
#define PN_OBS_LBA_BITMAP	"LBAbitmap"
#define PN_OBS_HBA_BITMAP	"HBAbitmap"

// BeamControl
#define PSN_BEAM_CONTROL	"LOFAR_ObsSW_@observation@_BeamControl"
#define PST_BEAM_CONTROL	"BeamControl"
#define PN_BC_CONNECTED	"connected"
#define PN_BC_SUB_ARRAY	"subArray"
#define PN_BC_SUBBAND_LIST	"subbandList"
#define PN_BC_BEAMLET_LIST	"beamletList"
#define PN_BC_ANGLE1	"angle1"
#define PN_BC_ANGLE2	"angle2"
#define PN_BC_DIRECTION_TYPE	"directionType"
#define PN_BC_BEAM_NAME	"beamName"

// CalibrationControl
#define PSN_CALIBRATION_CONTROL	"LOFAR_ObsSW_@observation@_CalibrationControl"
#define PST_CALIBRATION_CONTROL	"CalibrationControl"
#define PN_CC_CONNECTED	"connected"
#define PN_CC_BEAM_NAMES	"beamNames"
#define PN_CC_ANTENNA_ARRAY	"antennaArray"
#define PN_CC_FILTER	"filter"
#define PN_CC_NYQUISTZONE	"nyquistzone"
#define PN_CC_RCUS	"rcus"

// TBBControl
#define PSN_TBB_CONTROL	"LOFAR_ObsSW_@observation@_TBBControl"
#define PST_TBB_CONTROL	"TBBControl"
#define PN_TBC_CONNECTED	"connected"
#define PN_TBC_TRIGGER_RCU_NR	"trigger.rcuNr"
#define PN_TBC_TRIGGER_SEQUENCE_NR	"trigger.sequenceNr"
#define PN_TBC_TRIGGER_TIME	"trigger.time"
#define PN_TBC_TRIGGER_SAMPLE_NR	"trigger.sampleNr"
#define PN_TBC_TRIGGER_SUM	"trigger.sum"
#define PN_TBC_TRIGGER_NR_SAMPLES	"trigger.nrSamples"
#define PN_TBC_TRIGGER_PEAK_VALUE	"trigger.peakValue"
#define PN_TBC_TRIGGER_FLAGS	"trigger.flags"
#define PN_TBC_TRIGGER_TABLE	"trigger.table"
#define PN_TBC_TRIGGER_MISSED	"trigger.missed"

#endif
