    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
    #ifndef _TypeIter_Meq_h
    #define _TypeIter_Meq_h 1



#define DoForAllOtherTypes_Meq(Do,arg,separator) \
        

#define DoForAllBinaryTypes_Meq(Do,arg,separator) \
        

#define DoForAllSpecialTypes_Meq(Do,arg,separator) \
        

#define DoForAllIntermediateTypes_Meq(Do,arg,separator) \
        

#define DoForAllDynamicTypes_Meq(Do,arg,separator) \
        Do(Meq::Domain,arg) separator \
        Do(Meq::Cells,arg) separator \
        Do(Meq::VellSet,arg) separator \
        Do(Meq::Result,arg) separator \
        Do(Meq::Polc,arg) separator \
        Do(Meq::Node,arg) separator \
        Do(Meq::Function,arg) separator \
        Do(Meq::Constant,arg) separator \
        Do(Meq::Parm,arg) separator \
        Do(Meq::Freq,arg) separator \
        Do(Meq::Time,arg) separator \
        Do(Meq::Selector,arg) separator \
        Do(Meq::Composer,arg) separator \
        Do(Meq::Add,arg) separator \
        Do(Meq::Subtract,arg) separator \
        Do(Meq::Multiply,arg) separator \
        Do(Meq::Divide,arg) separator \
        Do(Meq::Sin,arg) separator \
        Do(Meq::Cos,arg) separator \
        Do(Meq::Exp,arg) separator \
        Do(Meq::Pow,arg) separator \
        Do(Meq::Sqr,arg) separator \
        Do(Meq::Sqrt,arg) separator \
        Do(Meq::Conj,arg) separator \
        Do(Meq::ToComplex,arg) separator \
        Do(Meq::UVW,arg) separator \
        Do(Meq::Request,arg) separator \
        Do(Meq::Condeq,arg) separator \
        Do(Meq::Solver,arg) separator \
        Do(Meq::ZeroFlagger,arg) separator \
        Do(Meq::MergeFlags,arg) separator \
        Do(Meq::Resampler,arg)

#define DoForAllNumericTypes_Meq(Do,arg,separator) \
        
#endif
