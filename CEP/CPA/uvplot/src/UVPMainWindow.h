// Copyright notice should go here

// $ID$

#if !defined(MAINWIN_H)
#define MAINWIN_H


//#include <qwidget.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qlabel.h>

#include <UVPUVCoverageArea.h>




class Tmain_window:public QMainWindow
{
  Q_OBJECT                      // to make the signal/slot mechanism work

 public:                        /* Public part */

  enum e_menu_command{mc_open, mc_quit, mc_help, mc_information};

  
   Tmain_window();
  ~Tmain_window();

 protected:                     /* Protected part */
  
  QMenuBar*       m_menu_bar;
  QPopupMenu*     m_file_menu;
  QPopupMenu*     m_view_menu;
  QPopupMenu*     m_help_menu;
  
  QStatusBar*     itsStatusBar;
  QProgressBar*   itsProgressBar; /* Resides in Status bar */
  QLabel*         itsXPosLabel;
  QLabel*         itsYPosLabel;

  UVPUVCoverageArea* itsCanvas;      /* The drawing canvas */
  UVPImageCube*   itsCube;


  virtual void resizeEvent(QResizeEvent *event);

  protected slots:

    // Display the world coordinates of the mouse pointer in the statusbar
    void slot_mouse_world_pos(double x, double y);

    void slot_about_uvplot();
};

#endif // MAINWIN_H
