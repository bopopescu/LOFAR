#include "AID-Basic.h"
#include "DataRecord.h"
    
#define paddr(x) printf("=== " #x ": %08x\n",(int)&x)
    
void TestFunc( const LockedBlockRef &ref )
{
  cerr<<"======================= Copying ref in function\n";
  ref.copy();
}

void TestCountedRefs()
{
  cerr<<"=============================================\n";
  cerr<<"======================= Testing CountedRefs =\n";
  cerr<<"=============================================\n\n";

  cerr<<"======================= Allocating two blocks\n";
  SmartBlock block1(0x2000);
  SmartBlock * block2 = new SmartBlock(0x2000);

  cerr<<"======================= attaching ref1 to block1\n";
  BlockRef ref1( block1,DMI::NON_ANON|DMI::WRITE );
  cerr<<"======================= attaching ref2/3 to block2\n";
  BlockRef ref2( block2,DMI::ANON|DMI::WRITE ),
           ref3( block2,DMI::ANON|DMI::READONLY );
  paddr(ref1);
  paddr(ref2);
  paddr(ref3);

  cerr<<"======================= ref1 -> ref1a (copy constructor)\n";
  BlockRef ref1a(ref1,DMI::COPYREF|DMI::WRITE);
  paddr(ref1a);
  Assert1( block1.refCount() == 2 );
  cerr<<"======================= ref1a -> ref1b (copy() method)\n";
  BlockRef ref1b = ref1a.copy();
  paddr(ref1b);
  Assert1( block1.refCount() == 3 );
  cerr<<"======================= ref1b -> ref1c (xfer constructor)\n";
  BlockRef ref1c(ref1b);
  paddr(ref1c);
  Assert1( block1.refCount() == 3 );

  cerr<<"======================= privatizing ref3\n";
  ref3.privatize();
  Assert1( block2->refCount() == 1 );
  Assert1( ref3.deref().refCount() == 1);
  cerr<<"======================= passing ref3 to function taking a LockedRef\n";
  TestFunc(ref3);

  cerr<<"======================= ref1a -> ref1d (copy(PERSIST))\n";
  BlockRef ref1d = ref1a.copy(DMI::PERSIST);
  cerr<<"======================= ref1d -> ref1e (= operator)\n";
  BlockRef ref1e = ref1d;

  cerr<<"======================= copying ref2 -> ref2a (copy(PRESERVE_RW))\n";
  BlockRef ref2a = ref2.copy(DMI::PRESERVE_RW);
  Assert1( block2->refCount() == 2 );
  Assert1( ref2a.isWritable() );
  cerr<<"======================= ref2a.privatize(DMI::WRITE|DMI::DLY_CLONE): no snapshot expected\n";
  ref2a.privatize(DMI::WRITE|DMI::DLY_CLONE);
  Assert1( block2->refCount() == 2 );
  cerr<<"======================= dereferencing ref2a\n";
  const SmartBlock *bl = &ref2a.deref();
  Assert1( bl != block2 );
  Assert1( block2->refCount() == 1 );
  Assert1( bl->refCount() == 1 );
  cerr<<"======================= exiting CountedRef Block\n";
}

void TestDataField ()
{
  cerr<<"=============================================\n";
  cerr<<"======================= Testing DataField   =\n";
  cerr<<"=============================================\n\n";

  cerr<<"======================= allocating empty field\n";
  DataField f1;
  cerr<<f1.sdebug(2)<<endl;

  cerr<<"======================= allocating field of 32 ints\n";
  DataField f2(Tpint,32);
  f2[0] = 1;
  f2[15] = 2;
  cerr<<f2.sdebug(2)<<endl;
  for( int i=0; i<32; i++ )
    cerr<<f2[i].as_double()<<" ";
  cerr<<endl;

  cerr<<"======================= converting to block: \n";
  BlockSet set;
  cerr<<"toBlock returns "<<f2.toBlock(set)<<endl;
  cerr<<"and set: "<<set.sdebug(1)<<endl;

  cerr<<"======================= building from block: \n";
  DataField f2a;
  cerr<<"Empty field allocated\n";
  cerr<<"fromBlock returns "<<f2a.fromBlock(set)<<endl;
  cerr<<"remaining set: "<<set.sdebug(1)<<endl;
  cerr<<"resulting field is: "<<f2a.sdebug(2)<<endl;
  for( int i=0; i<32; i++ )
    cerr<<f2a[i].as_float()<<" ";
  cerr<<endl;
  cerr<<"======================= exiting and destroying:\n";
}
    

void TestDataRecord ()
{
  cerr<<"=============================================\n";
  cerr<<"======================= Testing DataRecord   =\n";
  cerr<<"=============================================\n\n";

  cerr<<"======================= allocating empty record\n";
  DataRecord rec;

  cerr<<"======================= allocating field of 32 ints\n";
  DataFieldRef f2(new DataField(Tpint,32),DMI::ANON|DMI::WRITE); 
  DataField &f = f2.dewr();
  f[0] = 1;
  f[15] = 2.5;
  cerr<<f2->sdebug(2)<<endl;
  for( int i=0; i<32; i++ )
    cerr<<(float)(f[i])<<" ";
  cerr<<endl;
  cerr<<"======================= adding to record\n";
  HIID id("A.B.C.D");
  cerr<<"ID: "<<id.toString()<<endl;
  rec.add(id,f2,DMI::COPYREF|DMI::WRITE);
  rec["A.B.C.D"][20] = 5;
  cerr<<(int)(rec["A.B.C.D"][20])<<" "<<(int*)&(rec["A.B.C.D"][20])
      <<"  "<<rec["A.B.C.D"].as_int_p();
  cerr<<"======================= record debug info:\n";
  cerr<<rec.sdebug(3)<<endl;
  cerr<<"======================= old field debug info:\n";
  cerr<<f2->sdebug(3)<<endl;
  
  cerr<<"======================= making compound record\n";
  rec.add(AidB,new DataField(TpDataRecord,1));
  rec.add(AidC,f2,DMI::COPYREF);
  rec.add(AidD,f2,DMI::COPYREF);
  cerr<<"===== added subrecord B\n"<<rec.sdebug(3)<<endl;
  rec["B"]->add(AidC,new DataField(TpDataRecord,1));
  cerr<<"===== added subrecord B.C\n"<<rec.sdebug(10)<<endl;
  ((DataRecord&)(rec["B"]))["C"]->add(AidA,new DataField(Tpint,32));
  cerr<<"Record is "<<rec.sdebug(10)<<endl;
  
  cerr<<"======================= converting record to blockset\n";
  BlockSet set;
  rec.toBlock(set);
  cerr<<"blockset is: "<<set.sdebug(2)<<endl;
  
  cerr<<"======================= loading record from blockset\n";
  DataRecord rec2;
  rec2.fromBlock(set);
  cerr<<"New record is "<<rec2.sdebug(10)<<endl;
  cerr<<"Blockset now "<<set.sdebug(2)<<endl;

  cerr<<"======================= accessing cached field\n";
  cerr<<"Value: "<<rec2["B.C.A.10"].as_double()<<endl;
  
  cerr<<"======================= exiting\n";
}

int main ( int argc,const char *argv[] )
{
  Debug::DebugContext.setLevel(10);
  CountedRefBase::DebugContext.setLevel(10);
  
  Debug::initLevels(argc,argv);
  
  try 
  {
    TestCountedRefs();
    TestDataField();
    TestDataRecord();
  }
  catch( Debug::Error err ) 
  {
    cerr<<"\nCaught exception:\n"<<err.what()<<endl;
    return 1;
  }

  return 0;
}
    
    
    
