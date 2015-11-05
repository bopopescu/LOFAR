package org.astron.lofarsim;

import org.astron.basesim.GraphWorkHolderImage;
import org.astron.basesim.*;
import org.astron.basesim.def.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

public class WHFFT extends GraphWorkHolderImage {

  public WHFFT() {
    super();
  }

  public String getClassName() { return "FFT"; }
  public GraphDataHolder buildDataHolder(boolean input) {
    if (input) {
      return new DHAntenna();
    } else {
      return new DHFreq();
    }
  }

}