// Copyright notice should go here

#if !defined(UVPTIMEFREQUENCYPLOT_H)
#define UVPTIMEFREQUENCYPLOT_H

// $Id$

#include <UVPDisplayArea.h>
#include <UVPSpectrumVector.h>


#include <vector>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

class UVPTimeFrequencyPlot: public UVPDisplayArea
{
  Q_OBJECT
  
#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif
    
 public:
  
                 UVPTimeFrequencyPlot(QWidget *parent);
  

  virtual void drawView();
  
  void         setChannels(unsigned int numberOfChannels);

 public slots:
  
  // Add a spectrum to itsSpectrum.
  void slot_addSpectrum(const UVPSpectrum &spectrum);
  void slot_paletteChanged();
  
  
 protected:
 private:

  UVPSpectrumVector  itsSpectrum;
  
  UVPAxis            itsValueAxis;
};

#endif // UVPTIMEFREQUENCYPLOT_H
