// Copyright notice should go here

#include <uvplot/UVPDataAtomVector.h>


//===============>>>  UVPDataAtomVector::UVPDataAtomVector  <<<===============

UVPDataAtomVector::UVPDataAtomVector()
  : std::vector<const UVPDataAtom *>(),
    itsMinRe(0),
    itsMaxRe(0),
    itsMinIm(0),
    itsMaxIm(0)
{
}




//===============>>>  UVPDataAtomVector::~UVPDataAtomVector  <<<===============

UVPDataAtomVector::~UVPDataAtomVector()
{
}



//===============>>>  UVPDataAtomVector::minRe  <<<===============

double UVPDataAtomVector::minRe() const
{
  return itsMinRe;
}





//===============>>>  UVPDataAtomVector::maxRe  <<<===============

double UVPDataAtomVector::maxRe() const
{
  return itsMaxRe;
}




//===============>>>  UVPDataAtomVector::minIm  <<<===============

double UVPDataAtomVector::minIm() const
{
  return itsMinIm;
}






//===============>>>  UVPDataAtomVector::maxIm  <<<===============

double UVPDataAtomVector::maxIm() const
{
  return itsMaxIm;
}




//===============>>>  UVPDataAtomVector::add  <<<===============

void UVPDataAtomVector::add(const UVPDataAtom *atom)
{
  push_back(atom);

  double MinRe(0);
  double MaxRe(0);
  double MinIm(0);
  double MaxIm(0);

  const UVPDataAtom::ComplexType* value = atom->getData(0);
  unsigned int                    N     = atom->getNumberOfChannels();
  const UVPDataAtom::ComplexType* end   = value + N;

  if(N > 0) {
    MinRe = (*value).real();
    MaxRe = MinRe;
    MinIm = (*value).imag();
    MaxIm = MinIm;
    value++;

    while(value < end) {
      double real = (*value).real();
      double imag = (*value).imag();
      value++;

      MinRe = ( real < MinRe ? real : MinRe);
      MaxRe = ( real > MaxRe ? real : MaxRe);
      MinIm = ( imag < MinIm ? imag : MinIm);
      MaxIm = ( imag > MaxIm ? imag : MaxIm);
    }
    
    if(size() == 1) {
      itsMinRe = MinRe;
      itsMaxRe = MaxRe;
      itsMinIm = MinIm;
      itsMaxIm = MaxIm;
    } else {
      if(MinRe < itsMinRe) {
        itsMinRe = MinRe;
      }
      if(MaxRe > itsMaxRe) {
        itsMaxRe = MaxRe;
      }
      if(MinIm < itsMinIm) {
        itsMinIm = MinIm;
      }
      if(MaxIm > itsMaxIm) {
        itsMaxIm = MaxIm;
      }
    } // If size() ...

  } // If N > 0
  
}


