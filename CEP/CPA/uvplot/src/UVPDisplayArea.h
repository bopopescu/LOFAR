// Copyright notice should go here

#if !defined(UVPDISPLAYAREA_H)
#define UVPDISPLAYAREA_H

// $Id$


#include <vector>


#include <qwidget.h>
#include <qpixmap.h>

#include <uvplot/UVPAxis.h>



class UVPDisplayArea : public QWidget
{
  Q_OBJECT
    
 public:                        /* Public part */

               UVPDisplayArea(QWidget *    parent,
                              unsigned int numColors=256);
              ~UVPDisplayArea();

 void          initColormap(double slope,
                            double center);

 unsigned int  getNumberOfColors() const;
 inline const QColor *getColor(unsigned int color) const;

 const UVPAxis *getXAxis() const;
 const UVPAxis *getYAxis() const;
 
 void  setXAxis(const UVPAxis &axis);
 void  setYAxis(const UVPAxis &axis);


 virtual void drawView();


 signals:

 // Emitted when mouse position changes
 void signal_mouseWorldPosChanged(double x,
                                  double y);
 
 void signal_paletteChanged();

 protected:                     /* Protected part */

  QPixmap      itsBuffer;

  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);

 private:
  
  std::vector<QColor> itsColormap;
  UVPAxis             itsXAxis;
  UVPAxis             itsYAxis;

  std::vector<QColor> itsComplexColormap;
  std::vector<int>    itsRealIndex;
  std::vector<int>    itsImagIndex;
};





//====================>>>  UVPDisplayArea::getColor  <<<====================

inline const QColor *UVPDisplayArea::getColor(unsigned int color) const
{
  return &(itsColormap[color]);
}


#endif // UVPDISPLAYAREA_H
