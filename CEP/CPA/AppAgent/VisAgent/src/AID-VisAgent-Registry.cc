    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/src/build_aid_maps.pl
    #include <DMI/AtomicID.h>
    #include <DMI/TypeInfo.h>
    #include <DMI/DynamicTypeManager.h>
    #include <DMI/Packer.h>
    
  
    int aidRegistry_VisAgent ()
    {
      static int res = 

        AtomicID::registerId(-1116,"Vis")+
        AtomicID::registerId(-1075,"Input")+
        AtomicID::registerId(-1130,"Output")+
        AtomicID::registerId(-1249,"Agent")+
        AtomicID::registerId(-1282,"Parameters")+
        AtomicID::registerId(-1107,"Data")+
        AtomicID::registerId(-1133,"Header")+
        AtomicID::registerId(-1202,"Footer")+
        AtomicID::registerId(-1123,"Tile")+
        AtomicID::registerId(-1252,"Suspend")+
        AtomicID::registerId(-1251,"Resume")+
    0;
    return res;
  }
  
  int __dum_call_registries_for_VisAgent = aidRegistry_VisAgent();

