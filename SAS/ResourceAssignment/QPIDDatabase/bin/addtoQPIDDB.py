#!/usr/bin/python

# setup PYTHONPATH while testing: export PYTHONPATH=../lib:$PYTHONPATH
import sys
from QPIDDB import qpidinfra


from optparse import OptionParser






if __name__ == '__main__':
    parser = OptionParser('%prog [options]',
	     description='Add items to the QPIDinfra database from the commandline')
    parser.add_option('-b', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker (required)')
    parser.add_option('-f', '--federation', dest='federation' , type='string', default=None, help='Address of the federated broker')
    parser.add_option('-q', '--queue', dest='queue', type='string', default=None, help='Name of the queue on the broker')
    parser.add_option('-e', '--exchange', dest='exchange', type='string', default=None, help='Name of the exchange on the broker')
    parser.add_option('-k', '--routingkey', dest='routingkey', type='string', default='#', help='Federation routing key')

    (options, args) = parser.parse_args()

    if (len(sys.argv)<2):
	 parser.print_help()

    QPIDinfra = qpidinfra()

    if (options.broker==None):
	parser.print_help()

    else:
	QPIDinfra.addhost(options.broker)

    if (options.queue):
	QPIDinfra.addqueue(options.queue)
	QPIDinfra.bindqueuetohost(options.queue,options.broker)
    
    if (options.exchange):
	QPIDinfra.addexchange(options.exchange)
	QPIDinfra.bindexchangetohost(options.exchange,options.broker)
    
    if (options.federation):
	QPIDinfra.addhost(options.federation)
	if (options.queue):
	    QPIDinfra.addqueue(options.queue) # should be superfluous
	    ecxchange=''
	    if (options.exchange):
		QPIDinfra.addexchange(options.exchange)
		exchange=options.exchange
	    
	    QPIDinfra.bindqueuetohost(options.queue,options.federation)
	    QPIDinfra.setqueueroute(options.queue,options.broker,options.federation,exchange)
	else:
	    if (options.exchange):
	        QPIDinfra.addexchange(options.exchange) # should be superfluous
	        QPIDinfra.bindexchangetohost(options.exchange,options.federation)
	        QPIDinfra.setexchangeroute(options.exchange,options.routingkey,options.broker,options.federation)
	    else:
		raise Exception("federation can only be setup with a queue or an exchange")

