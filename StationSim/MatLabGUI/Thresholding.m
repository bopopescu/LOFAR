MainFigPos_Thres = [ 50 500 300 150];
shh = get(0,'ShowHiddenHandles');
set(0,'ShowHiddenHandles','on')
%Figure principale
HMainFig_Thres = figure(...
   'Name','Threshold Window', ...
   'handlevisibility','callback',...
   'IntegerHandle','off',...
   'NumberTitle','off',...
   'Tag','MainFig2_thres',...
   'position',MainFigPos_Thres);

frm1PosThres=[LeftFrameBorder*0.008 BottomBorder FrameWidth*8 FrameHeight ];
frm1Perio=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm1PosThres, ...
   'BackgroundColor',CompBackGroundColor );

frm1PosThres=[LeftFrameBorder*0.045 BottomBorder*2 FrameWidth*7.5 FrameHeight*0.9 ];
frm1Perio=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm1PosThres, ...
   'BackgroundColor',[0.6 0.5 0.5] );

ComPos50=[];
ComPos50=[LeftFrameBorder*0.75 CompBottom*0.75 CompWidth*5 CompHeight*6];
Thres_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos50, ...
   'String','Manual Threshold(dB)');

ComPos511=[];
ComPos511=[LeftFrameBorder*0.75 CompBottom*0.57 CompWidth*5 CompHeight*5];
Thres=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos511, ...
   'String','-60');


ComPos601=[];
ComPos601=[LeftFrameBorder*0.15 CompBottom*0.75 CompWidth*9 CompHeight*5];
RadioThres1=uicontrol( ...
   'Style','radiobutton', ...
   'Units','normalized', ...
   'Position',ComPos601, ...
   'String','Automatic Threshold','value',1,'BackgroundColor',[0.6 0.5 0.5],'Callback','radio1_thres');

ComPos611=[];
ComPos611=[LeftFrameBorder*0.15 CompBottom*0.57 CompWidth*9 CompHeight*5];
RadioThres2=uicontrol( ...
   'Style','radiobutton', ...
   'Units','normalized', ...
   'Position',ComPos611, ...
   'String','Manual Threshold','BackgroundColor',[0.6 0.5 0.5],'Callback','radio2_thres');

if indice_thres==1
Callback_choice='Apply_Thres';
else
Callback_choice='Apply_Thres_FFT';   
end
ComPos521=[];
ComPos521=[LeftFrameBorder*0.15 CompBottom*0.25 CompWidth*4 CompHeight*6];
Apply_thresButton=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos521, ...
   'String','Apply','Callback',Callback_choice);

ComPos541=[];
ComPos541=[LeftFrameBorder*0.80 CompBottom*0.25 CompWidth*4 CompHeight*6];
Apply_thresSave=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos541, ...
   'String','Save','Callback','save_reference');

ComPos542=[];
ComPos542=[LeftFrameBorder*0.48 CompBottom*0.25 CompWidth*4 CompHeight*6];
Apply_thresSurf=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos542, ...
   'String','Surf plot','Callback','surf_threshold');

ComPos531=[];
ComPos531=[LeftFrameBorder*0.40 CompBottom CompWidth*8 CompHeight*3];
title=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos531, ...
   'String','Threshold Value','BackgroundColor',CompBackGroundColor);

