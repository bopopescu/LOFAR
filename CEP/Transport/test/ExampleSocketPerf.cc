//#  ExampleSocket.cc: a test program for the TH_Socket class
//#                    this program sends a series of packages 
//#                    with increasing package size and measures
//#                    the transport bandwidth
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>
#include <Transport/TH_Socket.h>
#include <DH_Example.h>
#include <StopWatch.h>

using namespace LOFAR;

void displayUsage (void);

int main (int argc, char** argv) {
  bool isReceiver; 
  // isReceiver == true  => this program must run as a server (receiver).
  // isReceiver == false => this program must run as a client (sender).

  cout << "argc = " << argc << endl;
  if (argc < 3) {
    displayUsage ();
    return 0;
  }
  cout << "argc = " << argc << endl;

  bool ServerAtSender=false; // flag for who should be the server

  if (! strcmp (argv [1], "-r")) {
    cout << "(Receiver side)" << endl;
    isReceiver = true;
    if (argc >= 3  && !strcmp(argv[2], "-Server")) ServerAtSender=false;
    if (argc >= 3  && !strcmp(argv[2], "-Client")) ServerAtSender=true;
  } else if (! strcmp (argv [1], "-s")) {
    cout << "(Sender side)" << endl;
    isReceiver = false;
    if (argc >= 3 && !strcmp(argv[2], "-Server"))  ServerAtSender=true;
    if (argc >= 3 && !strcmp(argv[2], "-Client"))  ServerAtSender=false;
  } else {
    displayUsage ();
    return 0;
  }


   const int maxlen = 1024*1024*1;
   DH_Example DH_Sender("dh1", 
			100+maxlen*sizeof(DH_Example::BufferType));  //size
  DH_Example DH_Receiver("dh",
			 100+maxlen*sizeof(DH_Example::BufferType));  //size

  DH_Sender.setID(1);
  DH_Receiver.setID(2);

  DH_Sender.setBlocking(true);
  DH_Receiver.setBlocking(true);

  TH_Socket TH_proto("lofar11", 
		     "lofar12",
		     8923,
		     ServerAtSender); //  set the server side

  DH_Sender.connectTo (DH_Receiver, TH_proto);
  DH_Sender.init();
  DH_Receiver.init();
 
   
   
  if (! isReceiver) {
    // fill the DataHolders with some initial data
    DH_Sender.getBuffer()[0] = fcomplex(17,-3.5);
    DH_Sender.getBuffer()[1] = fcomplex(18,-2.5);
    DH_Receiver.getBuffer()[0] = 0;
    DH_Receiver.getBuffer()[1] = 0;
    DH_Sender.setCounter(2);
    DH_Receiver.setCounter(0);
    cout << "Before transport : " 
	 << DH_Sender.getBuffer()[1] << ' ' << DH_Sender.getCounter()
	 << " -- " 
	 << DH_Receiver.getBuffer()[1] << ' ' << DH_Receiver.getCounter()
	 << endl;
  }


  StopWatch watch;
  watch.stop();


  double sizes[10000];
  double times[10000];
  int i=0;

  for (int l=1; l<maxlen; l= (int)ceil(1.051*l)) {

    sizes[i]=(double)l;
    // resize
    if (isReceiver) {
      DH_Receiver.setCurDataSize(l); 
    } else {
      DH_Sender.setCurDataSize(l); 
    }
    
    // perform measurement
    // 10-1000 iterations are done depending on the pkg size
    // the average transport time is calculated on the send side
    int measurements=10;
    if (l< 256*1024) measurements = 100;
    if (l<  16*1024) measurements = 1000;
    if (l<   1*1024) measurements = 10000;

    //    cout << "len = " << l << "  meas: " << measurements << endl;
    
    if (!isReceiver) watch.start();
    for (int m=0; m< measurements; m++) {
      if (isReceiver) {
        DH_Receiver.read();
      } else {
	DH_Sender.write();
      }
    }
    if (!isReceiver) {
      watch.stop();
      times[i++] = watch.elapsed()/measurements;
    }
  }
  
  if (!isReceiver)  {
    ofstream outFile("bandwidth.dat");
    for (int m=0; m<i; m++) {
      outFile << sizes[m] << "  "              // packet size in bytes/
	      << log(sizes[m]) << "  "               // log packet size 
	      << times[m] << "  "              // time interval
	      << sizes[m] /1024. / 1024. / times[m]  // bandwidth in MB/sec
	      << endl;
    }
    outFile.close();
  }
  
  
  //    note that transport is bi-directional.
  // so this will also work:
  //   DH_Receiver.write();
  //   DH_Sender.read();
  // 
  
  if (isReceiver) {
    cout << " After transport  : " 
	 << DH_Sender.getBuffer()[1] << ' ' << DH_Sender.getCounter()
	 << " -- " 
	 << DH_Receiver.getBuffer()[1] << ' ' << DH_Receiver.getCounter()
	 << endl;
    /*
      if (DH_Sender.getBuffer()[0] == DH_Receiver.getBuffer()[0]
      &&  DH_Sender.getCounter() == DH_Receiver.getCounter()) {
      }
    */
  }
  return 0;
}


void displayUsage (void) {
    cout << "Usage: ExampleSocket -s|-r [-Server|-Client]" << endl;
    cout << "If Client/Server flags are not specified, the receiver side will be Server" << endl;
    cout << "(exit)." << endl;
}
