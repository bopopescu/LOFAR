// 
//  ETH_realtime.cc: test to see how we can use raw Ethernet
//                   as real-time transport medium.
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include "DataEvent.h"

#include <getopt.h>
#include <iostream>

#include <sys/time.h>

#define REQUEST_SIZE 1030
#define RESPONSE_SIZE 42

#define MAX_IFNAME_LEN 64
#define HWADDR_LEN 17

using namespace std;

class ETH_realtime : public GCFTask
{
public:

    ETH_realtime(const char* name, bool isClient,
		 const char* ifname, const char* dest_mac = "");

private:

    int initial_state  (GCFEvent& e, GCFPortInterface& port);
    void send_message(unsigned long sequence_number, size_t size);

private:

    GCFETHRawPort channel;
    bool          m_isClient;
};

ETH_realtime::ETH_realtime(const char* name, bool isClient,
		   const char* ifname, const char* dest_mac) :
    GCFTask((State)&ETH_realtime::initial_state, name), channel(), m_isClient(isClient)
{
  // client or server?
  channel.init(*this, "channel", (isClient ? GCFPortInterface::SAP : GCFPortInterface::SPP), 0, true);

  channel.setAddr(ifname, dest_mac);
}

static char ETH_sendpacket[ETH_DATA_LEN];
static char ETH_recvpacket[ETH_DATA_LEN];
#define LOOPMAX 1000

int ETH_realtime::initial_state(GCFEvent& e, GCFPortInterface& /*port*/)
{
  int status = GCFEvent::HANDLED; // event not handled
  static unsigned long sequence_number = 0;
  static struct timeval start = {0,0}, stop = {0,0};
  static int packetcount = 0;

  switch (e.signal)
  {
      case F_INIT:
      {
	  // open the ctrl_ port
	  channel.open();
	  
	  // initialize buffers
	  memset(ETH_sendpacket, 0, ETH_DATA_LEN);
	  memset(ETH_recvpacket, 0, ETH_DATA_LEN);
      }
      break;
      
      case F_CONNECTED:
      {
	  cout << "connected" << endl;

	  if (m_isClient)
	  {
	      send_message(sequence_number++, REQUEST_SIZE);
	      channel.setTimer((long)0, 0, 1, 0);
	  }
      }
      break;

      case F_DATAIN:
      {
	  int n;

	  if ((n = channel.recv(ETH_recvpacket, ETH_DATA_LEN)) < 0)
	  {
	      perror("channel.recv");
	      exit(EXIT_FAILURE);
	  }
	  if (n == 0) status = GCFEvent::ERROR;
	  else
	  {
	      // server checks and responds
	      unsigned long* hdr = (unsigned long*)&ETH_recvpacket[0];

	      if (*hdr++ == 0xdeadbeaf)
	      {
#if 0
		  cout << "Received matching packet " << *hdr << "; ";
		  cout << "Received " << n << " bytes." << endl;
#endif

		  if (!m_isClient)
		  {
		      send_message(*hdr, 2*sizeof(unsigned long));
		  }
		  else
		  {
		      if (*hdr != sequence_number-1) cerr << "seqnr mismatch" << endl;

		      packetcount++;
		      // receive ack, stop timer
		      gettimeofday(&stop, 0);

#if 0
		      // print delay
		      double delay = (stop.tv_sec - start.tv_sec)*1.0e6 + (stop.tv_usec - start.tv_usec);
		      cout << "delay = " << delay << " microseconds" << endl;
#endif
		      // immediately send next packet
		      gettimeofday(&start, 0);
		      send_message(sequence_number++, REQUEST_SIZE);
		  }
	      }
	  }
      }
      break;

      //case F_DATAOUT:
      case F_TIMER:
      {
#if 0
	  gettimeofday(&start, 0);
	  send_message(sequence_number++, REQUEST_SIZE);
#else
	  cerr << "packets/sec = " << packetcount << endl;
	  packetcount = 0;
#endif
      }
      break;
      case F_DISCONNECTED:
      {
	  // reopen the port
	  channel.open();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }
  
  return status;
}

void ETH_realtime::send_message(unsigned long sequence_number, size_t size)
{
  int n;

  unsigned long* hdr = (unsigned long*)&ETH_sendpacket[0];
  *hdr++ = 0xdeadbeaf;
  *hdr++ = sequence_number;

  static DataEvent data;
  data.payload.setBuffer(ETH_sendpacket, size);

  if ((n = channel.send(data)) < 0)
  {
      perror("channel.send");
      exit(EXIT_FAILURE);
  }
#if 0
  cout << "Sent " << n << " bytes." << endl;
#endif
}

//
// Program usage
//
void usage(const char* progname)
{
  fprintf(stderr, "usage: %s\n"
	  "\t--if=<e.g. eth0>\n"
	  "\t[--client]\n"
	  "\t[--peer=<e.g. 00:11:22:33:44:55>]\n"
	  "\t[-h]\n",
	  progname);
}

//
// Contoller main function
//
int main(int argc, char* argv[])
{
  bool isClient = false;
  char ifname[MAX_IFNAME_LEN + 1];
  char dest_mac[HWADDR_LEN + 1];

  char progname[100];

  GCFTask::init(argc, argv);

  // remember program name
  strncpy(progname, argv[0], 100);

  dest_mac[0] = '\0';
  ifname[0] = '\0';

  while (1)
  {
    static struct option long_options[] = 
      {
	{ "client",       no_argument, 0, 'c' },
	{ "if",     required_argument, 0, 'i' },
        { "peer",   required_argument, 0, 'p' },
	{ "help",         no_argument, 0, 'h' },
	{ 0, 0, 0, 0 },
      };

    int option_index = 0;
    int c = getopt_long_only(argc, argv,
			     "ci:p:h", long_options, &option_index);
    
    if (c == -1) break;
    
    switch (c)
    {
      case 'c':
	isClient = true;
	break;
	
      case 'i':
	strncpy(ifname, optarg, MAX_IFNAME_LEN);
	break;
	
      case 'p':
	memset(dest_mac, 0, HWADDR_LEN + 1);
	strncpy(dest_mac, optarg, HWADDR_LEN);
	break;

      case 'h':
	usage(argv[0]);
	exit(EXIT_SUCCESS);
	break;

      default:
	printf ("?? getopt returned character code 0%o ??\n", c);
	break;
    }
  }

  if (optind < argc) {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    printf ("\n");
  }
  
  if (ifname[0] == '\0')
  {
      cerr << "Error: mandatory '--if' argument missing." << endl;
      usage(argv[0]);
      exit(EXIT_FAILURE);
  }

  ETH_realtime ethtask("ETH_realtime", isClient, ifname, dest_mac);
  ethtask.start();

  GCFTask::run();

  cerr << "Returned from GFTask::run()! should never happen." << endl;

  return 1;
}
