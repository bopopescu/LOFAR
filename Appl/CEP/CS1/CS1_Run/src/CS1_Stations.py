# inputnode macadresses
inputMACs = [
    '00:E0:81:31:B3:EB',
    '00:E0:81:31:B0:E1',
    '00:E0:81:31:B2:F5',
    '00:E0:81:31:DE:8B',
    '00:E0:81:29:20:37',
    '00:E0:81:31:DE:9B',
    '00:E0:81:34:8C:D1',
    '00:E0:81:34:9B:87',
    '00:E0:81:31:B3:27',
    '00:E0:81:31:DF:D5',
    '00:E0:81:34:92:11',
    '00:E0:81:31:B2:15'
    ]

# todo: this doesn't work yet with multiple rspboards per station
class Station(object):
    """
    Represents a real or virtual station. Position is a tuple of three floats
    representing ITRF x,y,z coordinates.
    """
    def __init__(self, name, inputNode, position, RSPBoardNumber, mac):
        self.name = name
        self.inputNode = inputNode
        self.position = position
        stationNumber = int(name.split('_')[0].split('CS')[-1])
	self.mac = mac
    def getName(self):
        return self.name
    def getInputNode(self):
        return self.inputNode
    def getPosition(self):
        return self.position
    def getSrcMAC(self):
        return self.mac
    def getDstMAC(self):
        return inputMACs[self.inputNode - 1]

#define the stations as a list so we can easily add them
#Keep the antenna positions in ITRF coordinates.
#Storing a position on earth given as (lon,lat,height)

CS10_dipole0  = [Station('CS10_dipole0' ,  1, (0.119881183707,  0.920274058886, 6364096.65549 ), 0, '10:fa:00:0a:01:00')]
CS10_dipole4  = [Station('CS10_dipole4' ,  1, (0.119888715288,  0.920277077416, 6364096.63359 ), 1, '10:fa:00:0a:01:01')]
CS10_dipole8  = [Station('CS10_dipole8' ,  2, (0.119879699401,  0.920273668267, 6364096.7055  ), 2, '10:fa:00:0a:01:02')]
CS10_dipole12 = [Station('CS10_dipole12',  3, (0.119885492973,  0.92027554014 , 6364096.62412 ), 3, '10:fa:00:0a:01:03')]
CS10_4dipoles = CS10_dipole0 + CS10_dipole4 + CS10_dipole8 + CS10_dipole12

CS08_dipole0  = [Station('CS08_dipole0' ,  4, (0.119868257653,  0.920301867601, 6364095.96819 ), 0, '10:fa:00:08:01:05')]
CS08_dipole4  = [Station('CS08_dipole4' ,  4, (0.119874875505,  0.920305714116, 6364095.8893  ), 1, '10:fa:00:08:01:07')]
CS08_dipole8  = [Station('CS08_dipole8' ,  5, (0.119897959763,  0.92029772671 , 6364096.05311 ), 2, '10:fa:00:08:01:0a')]
CS08_dipole12 = [Station('CS08_dipole12',  6, (0.119891341863,  0.920293880294, 6364096.13299 ), 3, '10:fa:00:08:01:0d')]
CS08_4dipoles = CS08_dipole0 + CS08_dipole4 + CS08_dipole8 + CS08_dipole12

CS01_dipole0  = [Station('CS01_dipole0' ,  7, (0.11985110257 ,  0.920247392852, 6364097.49438 ), 0, '10:fa:00:01:01:00')]
CS01_dipole4  = [Station('CS01_dipole4' ,  7, (0.119857719888,  0.920251239615, 6364097.81449 ), 1, '10:fa:00:01:01:01')]
CS01_dipole8  = [Station('CS01_dipole8' ,  8, (0.119864071239,  0.920247231952, 6364097.89768 ), 2, '10:fa:00:01:01:02')]
CS01_dipole12 = [Station('CS01_dipole12',  9, (0.11985745392 ,  0.920243385441, 6364097.97757 ), 3, '10:fa:00:01:01:03')]
CS01_4dipoles = CS01_dipole0 + CS01_dipole4 + CS01_dipole8 + CS01_dipole12

CS16_dipole0  = [Station('CS16_dipole0' , 10, (0.119958428004,  0.920274972606, 6364096.62375 ), 0, '10:fa:00:10:01:00')]
CS16_dipole4  = [Station('CS16_dipole4' , 10, (0.119965046099,  0.920278818828, 6364096.54488 ), 1, '10:fa:00:10:01:01')]
CS16_dipole8  = [Station('CS16_dipole8' , 11, (0.11997139711 ,  0.920274810823, 6364096.62707 ), 2, '10:fa:00:10:01:02')]
CS16_dipole12 = [Station('CS16_dipole12', 12, (0.119964779015,  0.920270965005, 6364097.40695 ), 3, '10:fa:00:10:01:03')]
CS16_4dipoles = CS16_dipole0 + CS16_dipole4 + CS16_dipole8 + CS16_dipole12

Fourstations_4dipoles = CS10_4dipoles + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles

CS10_centre0  = [Station('CS10_centre0' ,  1, (0.119880687209, 0.920274829586, 6364096.65669), 0, '10:fa:00:0a:01:00')]
CS10_centre1  = [Station('CS10_centre1' ,  1, (0.119889696308, 0.920277341969, 6364096.62516), 1, '10:fa:00:0a:01:01')]
CS10_centre2  = [Station('CS10_centre2' ,  2, (0.119878836045, 0.920272809664, 6364096.71911), 2, '10:fa:00:0a:01:02')]
CS10_centre3 =  [Station('CS10_centre3' ,  3, (0.119884676596, 0.920276007334, 6364096.63253), 3, '10:fa:00:0a:01:03')]
CS10_4centra = CS10_centre0 + CS10_centre1 + CS10_centre2 + CS10_centre3

CS08_centre0  = [Station('CS08_centre0' ,  4, (0.119869165515, 0.920301856536, 6364096.36841), 0, '10:fa:00:08:01:05')]
CS08_centre1  = [Station('CS08_centre1' ,  4, (0.119875783416, 0.920305703002, 6364096.28953), 1, '10:fa:00:08:01:07')]
CS08_centre2  = [Station('CS08_centre2' ,  5, (0.119898867625, 0.920297715596, 6364096.45334), 2, '10:fa:00:08:01:0a')]
CS08_centre3 =  [Station('CS08_centre3' ,  6, (0.119892249725, 0.920293869179, 6364096.53222), 3, '10:fa:00:08:01:0d')]
CS08_4centra = CS08_centre0 + CS08_centre1 + CS08_centre2 + CS08_centre3

CS01_centre0  = [Station('CS01_centre0' ,  7, (0.119852010383, 0.920247381584, 6364097.49361), 0, '10:fa:00:01:01:00')]
CS01_centre1  = [Station('CS01_centre1' ,  7, (0.119858627702, 0.920251228144, 6364097.41472), 1, '10:fa:00:01:01:01')]
CS01_centre2  = [Station('CS01_centre2' ,  8, (0.119864979052, 0.920247220481, 6364097.49691), 2, '10:fa:00:01:01:02')]
CS01_centre3 =  [Station('CS01_centre3' ,  9, (0.119858361733, 0.92024337397 , 6364097.5778 ), 3, '10:fa:00:01:01:03')]
CS01_4centra = CS01_centre0 + CS01_centre1 + CS01_centre2 + CS01_centre3

CS16_centre0  = [Station('CS16_centre0' , 10, (0.119959335866, 0.92027496129 , 6364096.62399), 0, '10:fa:00:10:01:00')]
CS16_centre1  = [Station('CS16_centre1' , 10, (0.119965953961, 0.920278807463, 6364096.54511), 1, '10:fa:00:10:01:01')]
CS16_centre2  = [Station('CS16_centre2' , 11, (0.119972304923, 0.920274799507, 6364096.6273 ), 2, '10:fa:00:10:01:02')]
CS16_centre3 =  [Station('CS16_centre3' , 12, (0.119965686829, 0.920270953334, 6364096.70718), 3, '10:fa:00:10:01:03')]
CS16_4centra = CS16_centre0 + CS16_centre1 + CS16_centre2 + CS16_centre3

Fourstations_4centra = CS10_4centra + CS01_4centra + CS08_4centra + CS16_4centra

