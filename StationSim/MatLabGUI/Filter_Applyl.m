OutputSignal = DFTFilterBankQuantised(signal,str2num(get(Filter_quant,'string')), str2num(get(IS_Quant,'string')), str2num(get(OS_Quant,'string')), FilterCoef1, str2num(get(Nb_Subband,'string')),, str2num(get(Order,'string')), 10000000000);