#!/usr/bin/env python

"""
The Thread class

$Id$

<todo>

This is not fully designed yet. The relation to the abstract classes
Assignment and CompositeAssignment on the one hand, and the concrete
classes Thread and Workload on the other hand needs more
consideration.

</todo>

"""

## import pg;
## db = pg.DB(dbname="bb");

import util.pgdate;
import util.pglist;


from CompositeAssignment import CompositeAssignment;
from Assignment import Assignment;
from Workload import Workload;
from Dataclass import Dataclass;

class Thread(CompositeAssignment,Dataclass):
  """a series of actions hopefully leading to a satisfying result"""
  
  root = 0;
  current = 0;
  workloads = [];

  def create_initial_workload(self,wl):
    iwl = Workload(wl);
    self.root = self.current = iwl;
    self.workloads.append(iwl);

  def addWl(self, wl):
    nwl = Workload(wl)
    self.current = nwl;
    self.workloads.append(self.current);

  def addWorkload(self, wl):
    wl.engine(self.current.engine());
    self.current = wl;
    self.workloads.append(self.current);

  def deleteWorkload(self,wl):
    c =  self.workloads.count(wl);
    i = 0;
    while i < c:
      self.workloads.remove(wl);
      i = i + 1;

  def addEngine(self,eng):
    self.current.engine(eng);
    

  def addAssignment(self, wl):
    self.current = wl;
    self.subAssignments.append(self.current);

  def __init__(self, wl):
    """initialize one self"""
    self.tablename = "threads"
    self.addAssignment(wl);
    self.create_initial_workload(wl);
    Dataclass.__init__(self);

  def evaluate():
    # """while reason to go on"""
    for i in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14):
      sleep(1);
      # """check last workload status"""
      # """decide on new workload"""
      # """yield"""

  def fork(self):
    """make a new thread"""

  def getCurrent(self):
    return self.current;
  
