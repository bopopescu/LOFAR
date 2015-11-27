#!/usr/bin/python
with open("add_virtual_instrument.sql", 'w') as output:

  output.write("-- resourceassignment password for testing on mcu005 is the same as the password on the president's luggage +6\n")
  output.write("-- psql resourceassignment -U resourceassignment -f add_virtual_instrument.sql -W\n")
  output.write("BEGIN;\n")
  #----- resource unit -----
  output.write("INSERT INTO virtual_instrument.unit VALUES \n")
  output.write("(0, 'rsp_channel_bit'),(1, 'bytes'),(2, 'rcu_board'),(3, 'bytes/second'),(4, 'cores');\n")
  #----- resource type -----
  output.write("INSERT INTO virtual_instrument.resource_type VALUES \n")
  rsp_index       = 0
  tbb_index       = 1
  rcu_index       = 2
  bandwidth_index = 3
  processor_index = 4
  storage_index   = 5
  output.write("(0, 'rsp', 0), (1, 'tbb', 1), (2, 'rcu', 2), (3, 'bandwidth', 3), (4, 'processor', 4), (5, 'storage', 1);\n")
  #----- resource_group type -----
  output.write("INSERT INTO virtual_instrument.resource_group_type VALUES \n")
  output.write("(0, 'instrument'),(1, 'cluster'),(2, 'station_group'),(3, 'station'),(4, 'node_group'),(5, 'node');\n")

  #----- resource groups ----- 
  output.write("INSERT INTO virtual_instrument.resource_group VALUES \n")
  ##instrument
  output.write("(0, 'LOFAR', 0),\n")
  ##cluster
  clusters = ['CEP2', 'CEP3', 'CEP4', 'Cobalt', 'DragNet']
  clusternumbers = range(1, 6)
  for i, cluster in enumerate(clusters):
    output.write("(%i, '%s', 1), " % (clusternumbers[i], cluster))
  output.write("\n")
  ##station group
  stationgroups = ['STATIONS', 'SUPERTERP', 'CORE', 'REMOTE', 'INTERNATIONAL', 'LORA', 'AARTFAAC']
  stationgroupnumbers = range(6, 13)
  for i, stationgroup in enumerate(stationgroups):
    output.write("(%i, '%s', 2), " % (stationgroupnumbers[i], stationgroup))
  output.write("\n")
  ##station this list should be imported from a central location?
  ##We might need to switch to CS001HBA1 CS003LBA notation at some point? This could also be a specification vs resource allocation problem.
  stations = ['CS001', 'CS002', 'CS003', 'CS004', 'CS005', 'CS006', 'CS007', 'CS011', 'CS013', 'CS017',
  'CS021', 'CS024', 'CS026', 'CS028', 'CS030', 'CS031', 'CS032', 'CS101', 'CS103', 'CS201', 'CS301',
  'CS302', 'CS401', 'CS501', 'RS106', 'RS205', 'RS208', 'RS210', 'RS305', 'RS306', 'RS307', 'RS310', 'RS406',
  'RS407', 'RS408', 'RS409', 'RS503', 'RS508', 'RS509', 'DE601', 'DE602', 'DE603', 'DE604', 'DE605', 'FR606',
  'SE607', 'UK608','DE609','PL610','PL611','PL612']
  stationnumbers = range(13, 64)
  for i, station in enumerate(stations):
    output.write("(%i, '%s', 3), " % (stationnumbers[i], station))
  output.write("\n")
  ##node
  cep4numbers = range(65, 114)
  for i in cep4numbers:
    output.write("(%i, 'cep4node', 5), " % (i,))
  output.write("\n")
  ##node_group
  output.write("(64, 'cep4_gpu', 4);\n") ## unknown which nodes are in here? Last entry for resource groups, to make the syntax work FIXME
  #----- resource & resource_to_resource_group & resource_capacity & resource_availability -----
  resource_count = 0
  resources = ""
  resource_to_resource_group = ""
  resource_capacity = ""
  resource_available = ""
  ## stations
  for station in stationnumbers:
    resources += "(%i, 'rsp', %i), " % (resource_count, rsp_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, station)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 61*16*4, 61*16*4) ##magic numbers FIXME
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
    resources += "(%i, 'tbb', %i), " % (resource_count, tbb_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, station)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 96*8*1024*1024*1024, 96*8*1024*1024*1024) ##magic numbers FIXME
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
    resources += "(%i, 'rcu', %i), " % (resource_count, rcu_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, station)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 96, 96) ##magic numbers FIXME, should be twice as high for INTERNATIONAL
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
    resources += "(%i, 'bandwidth', %i), " % (resource_count, bandwidth_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, station)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 3*1024*1024*1024, 3*1024*1024*1024) ##magic numbers FIXME
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
  ## nodes
  for node in cep4numbers:
    resources += "(%i, 'bandwidth', %i), " % (resource_count, bandwidth_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, node)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 50*1024*1024*1024, 50*1024*1024*1024) ##magic numbers FIXME
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
    resources += "(%i, 'processor', %i), " % (resource_count, processor_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, node)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 24, 24) ##magic numbers FIXME
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
    resources += "(%i, 'storage', %i), " % (resource_count, storage_index)
    resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, node)
    resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 0, 0) ##magic numbers FIXME
    resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
    resource_count += 1
  ## CEP4 bandwidth
  resources += "(%i, 'cep4bandwidth', %i), " % (resource_count, bandwidth_index)
  resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, 3) ##magic numbers FIXME
  resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 80*1024*1024*1024, 80*1024*1024*1024) ##magic numbers FIXME
  resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
  resource_count += 1
  ## CEP4 storage
  resources += "(%i, 'cep4storage', %i), " % (resource_count, storage_index)
  resource_to_resource_group += "(DEFAULT, %i, %i), " % (resource_count, 3) ##magic numbers FIXME
  resource_capacity += "(DEFAULT, %i, %i, %i), " % (resource_count, 2100*1024*1024*1024, 2100*1024*1024*1024) ##magic numbers FIXME
  resource_available += "(DEFAULT, %i, TRUE), " % (resource_count,)
  resource_count += 1
  
  output.write("INSERT INTO virtual_instrument.resource VALUES %s;\n" % resources[:-2])
  output.write("INSERT INTO virtual_instrument.resource_to_resource_group VALUES %s;\n" % resource_to_resource_group[:-2])
  output.write("INSERT INTO resource_monitoring.resource_capacity VALUES %s;\n" % resource_capacity[:-2])
  output.write("INSERT INTO resource_monitoring.resource_availability VALUES %s;\n" % resource_available[:-2])
  
  #----- resource_group_to_resource_group -----
  resource_group_to_resource_group = "(DEFAULT, 0, NULL), " ##LOFAR does not have parent, magic numbers FIXME
  for i, station in enumerate(stations):
    resource_group_to_resource_group += "(DEFAULT, %i, 6), " % (stationnumbers[i],) ## STATIONS
    if "CS" in station:
      resource_group_to_resource_group += "(DEFAULT, %i, 8), " % (stationnumbers[i],)  ## CORE
    elif "RS" in station:
      resource_group_to_resource_group += "(DEFAULT, %i, 9), " % (stationnumbers[i],)  ## REMOTE
    else:
      resource_group_to_resource_group += "(DEFAULT, %i, 10), " % (stationnumbers[i],)  ## INTERNATIONAL
    for node in cep4numbers:
      resource_group_to_resource_group += "(DEFAULT, %i, 3), " % (node,)  ## CEP4 nodes
  for cluster in clusternumbers:
    resource_group_to_resource_group += "(DEFAULT, %i, 0), " % (cluster,) # clusters to LOFAR
  for stationgroup in stationgroupnumbers:
    resource_group_to_resource_group += "(DEFAULT, %i, 0), " % (stationgroup,) # station groups to LOFAR
  output.write("INSERT INTO virtual_instrument.resource_group_to_resource_group VALUES %s;\n" % (resource_group_to_resource_group[:-2],))
  output.write("COMMIT;\n")
