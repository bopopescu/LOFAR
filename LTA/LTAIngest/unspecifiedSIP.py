#!/usr/bin/python

genericSIP = '''<?xml version="1.0" encoding="UTF-8"?>
<sip:ltaSip xmlns:sip="http://www.astron.nl/SIP-Lofar"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.astron.nl/SIP-Lofar LTA-SIP-2.6.0.xsd">
    <sipGeneratorVersion>Ingest (12-06-2015)</sipGeneratorVersion>
    <project>
        <projectCode>%s</projectCode>
        <primaryInvestigator>Unknown</primaryInvestigator>
        <coInvestigator>Unknown</coInvestigator>
        <contactAuthor>Unknown</contactAuthor>
        <telescope>LOFAR</telescope>
        <projectDescription>Unknown</projectDescription>
    </project>
    <dataProduct xsi:type="sip:UnspecifiedDataProduct">
        <dataProductType>Unknown</dataProductType>
        <dataProductIdentifier>
            <source>%s</source>
            <identifier>%s</identifier>
            <name>%s</name>
        </dataProductIdentifier>
        <storageTicket>%s</storageTicket>
        <size>%s</size>
        <checksum>
            <algorithm>MD5</algorithm>
            <value>%s</value>
        </checksum>
        <checksum>
            <algorithm>Adler32</algorithm>
            <value>%s</value>
        </checksum>
        <fileName>%s</fileName>
        <fileFormat>%s</fileFormat>
        <processIdentifier>
            <source>SAS</source>
            <identifier>%s</identifier>
            <name>Unknown process %s</name>
        </processIdentifier>
    </dataProduct>
    <unspecifiedProcess xsi:type="sip:UnspecifiedProcess">
        <processIdentifier>
            <source>SAS</source>
            <identifier>%s</identifier>
            <name>Unknown process %s</name>
        </processIdentifier>
        <observationId>
            <source>SAS</source>
            <identifier>%s</identifier>
        </observationId>
        <strategyName>Unknown</strategyName>
        <strategyDescription>Unknown</strategyDescription>
        <startTime>2010-01-01T00:00:00</startTime>
        <duration>PT0S</duration>
        <relations/>
        <observingMode>Unknown</observingMode>
        <description>Unknown process, information generated by Ingest</description>
    </unspecifiedProcess>
</sip:ltaSip>
'''


def makeSIP(Project, ObsId, MomId, ticket, FileName, FileSize, MD5Checksum, Adler32Checksum, Type):
  if FileName[-3:] == '.MS':
    fileFormat = 'AIPS++/CASA'
  elif FileName[-3:].lower() == '.h5':
    fileFormat = 'HDF5'
  elif FileName[-5:].lower() == '.fits':
    fileFormat = 'FITS'
  else: ## Maybe we need an 'Unknown' in the future?
    fileFormat = 'PULP'
  return genericSIP % (Project, Type, MomId, FileName, ticket, FileSize, MD5Checksum, Adler32Checksum, FileName, fileFormat, ObsId, ObsId, ObsId, ObsId, ObsId)

## Stand alone execution code ------------------------------------------
if __name__ == '__main__':
  usage = """Usage:
  unspecifiedSIP.py <config>
  TBD"""

#  if len(sys.argv) < 2:
#    print usage
#    exit(1)
  print makeSIP('test-lofar','12345','43213','VSN3FUNSP98N4F3NLSIWDUALFU3WDF','Bla.FITS',378964322,'Hoeba','Test')