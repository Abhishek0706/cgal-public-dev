// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s)     : Apurva Bhatt <response2apurva@gmail.com>
//The demo contains no error handling

#include <QApplication>
#include <qmessagebox.h>
#include <fstream>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QActionGroup>
#include <QtGui>
#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QSlider>
#include <QProgressBar>
#include <QMessageBox>
#include <fstream>   
#include <string>
#include <sstream>
#include <iomanip>
#include <list>

#include <boost/shared_ptr.hpp>   

#include <CGAL/basic.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Timer.h> 
#include <CGAL/Bbox_2.h>
#include <CGAL/iterator.h>
#include <CGAL/assertions_behaviour.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Lazy_exact_nt.h>
//#include <CGAL/Polygon_set_2.h>
//#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Iso_rectangle_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
//#include <CGAL/minkowski_sum_2.h>
#include <CGAL/approximated_offset_2.h>
#include <CGAL/Arrangement_2.h>
//#include <CGAL/Boolean_set_operations_2.h>
//#include <CGAL/Arr_circle_segment_traits_2.h>
//#include <CGAL/Arr_segment_traits_2.h>

#ifdef CGAL_USE_GMP
  #include <CGAL/Gmpq.h>
#else
  #include <CGAL/MP_Float.h>
  #include <CGAL/Quotient.h>
#endif

#include <QT5/Circular_polygons.h>
#include <QT5/Linear_polygons.h>
#include <QT5/Graphics_view_circular_polygon_input.h>
//#include <CGAL/Gps_segment_traits_2.h>
#include <QT5/Graphics_view_linear_polygon_input.h>


/*
#include <QT5/Piecewise_graphics_item_base.h>
#include <QT5/PiecewiseBoundaryGraphicsItem.h>
#include <QT5/PiecewiseRegionGraphicsItem.h>
#include <QT5/PiecewiseSetGraphicsItem.h>
*/
#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/DemosMainWindow.h>
#include <CGAL/Qt/utility.h>
#include <CGAL/IO/Dxf_bsop_reader.h>

#include <CGAL/Qt/GraphicsViewNavigation.h>

#include <QFileDialog>

#include "ui_Boolean_set_operations_2.h"

#include "Typedefs.h"

using namespace std;

typedef CGAL::Qt::Circular_set_graphics_item<Circular_polygon_set,Circular_traits> Circular_GI;
typedef CGAL::Qt::Linear_set_graphics_item<Linear_polygon_set,Linear_traits>     Linear_GI;

//Functions to show errors

void show_warning(std::string aS)
{
  QMessageBox::warning(NULL, "Warning", QString(aS.c_str()));
}

void show_error(std::string aS)
{
  QMessageBox::critical(NULL, "Critical Error", QString(aS.c_str()));
}

void error(std::string aS)
{
  show_error(aS);

  throw std::runtime_error(aS);
}

//****************************************************

//A way to maintain 3 set of polygons namely red,blue and result for all boolean operations
enum { BLUE_GROUP, RED_GROUP, RESULT_GROUP } ;

//A way to maintain 2 category of polygons namely linear,circular
//enum genrates errors so, we wil use LINEAR_TYPE=1, CIRCULAR_TYPE=2
//enum { LINEAR_TYPE, CIRCULAR_TYPE } ;

//dawing tools
QPen   sPens   [] = { QPen(QColor(0,0,255),0,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
                    , QPen(QColor(255,0,0),0,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)
                    , QPen(QColor(0,255,0),0,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) 
                    } ;
                    
QBrush sBrushes[] = { QBrush(QColor(0,0,255,32 ))
                    , QBrush(QColor(255,0,0,32 ))
                    , QBrush(QColor(0,255,0,220))
                    } ;
//**************************************

//A base call for rep class
struct Rep_base
{
  virtual ~Rep_base() {}
  
  virtual int type () const = 0 ;
   
  virtual CGAL::Qt::GraphicsItem* gi() const = 0 ;
  virtual CGAL::Qt::GraphicsItem* gi()       = 0 ;
  
  virtual void set_pen  ( QPen   const& aPen   ) = 0 ;
  virtual void set_brush( QBrush const& aBrush ) = 0 ;
  
  virtual QRectF bounding_rect() const { return gi()->boundingRect() ; }
  
  virtual bool is_empty() const = 0 ;
  
  virtual void clear               ()                         = 0 ;
  virtual void complement          ()                         = 0 ;
  virtual void assign              ( Rep_base const& aOther ) = 0 ;
  virtual void intersect           ( Rep_base const& aOther ) = 0 ;
  virtual void join                ( Rep_base const& aOther ) = 0 ;
  virtual void difference          ( Rep_base const& aOther ) = 0 ;
  virtual void symmetric_difference( Rep_base const& aOther ) = 0 ;
  
} ;


//Class for initializing 
template<class GI_, class Set_,class Gps_traits>
class Rep : public Rep_base
{
public:

  typedef GI_  GI  ;
  typedef Set_ Set ;
  typedef Rep<GI,Set,Gps_traits> Self ;
  typedef Gps_traits m_tratis;
  
  Rep() { m_GI = new GI(&m_set,m_traits) ; }
  
  Set const& set() const { return m_set ; }
  Set      & set()       { return m_set ; }
  
  virtual CGAL::Qt::GraphicsItem* gi() const { return m_GI; }
  virtual CGAL::Qt::GraphicsItem* gi()       { return m_GI; }
  
  virtual void set_pen  ( QPen   const& aPen   ) { m_GI->setPen  (aPen);   } 
  virtual void set_brush( QBrush const& aBrush ) { m_GI->setBrush(aBrush); }
      
  virtual bool is_empty() const { return m_set.is_empty() ; }
  
  virtual void clear()                         
  { 
    try
    {
      m_set.clear() ; 
    }
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    }
  }
  
  virtual void complement()                         
  { 
    try
    {
      m_set.complement(); 
    } 
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    } 
  }
  
  virtual void assign( Rep_base const& aOther ) 
  { 
    try
    {
      m_set = cast(aOther).m_set; 
    } 
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    } 
  }
  
  virtual void intersect( Rep_base const& aOther ) 
  { 
    try
    {
      m_set.intersection( cast(aOther).m_set); 
    } 
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    } 
  }
  
  virtual void join( Rep_base const& aOther ) 
  { 
    try
    {
      m_set.join( cast(aOther).m_set); 
    } 
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    } 
  }
  
  virtual void difference( Rep_base const& aOther ) 
  { 
    try
    {
      m_set.difference( cast(aOther).m_set); 
    } 
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    } 
  }
  
  virtual void symmetric_difference( Rep_base const& aOther ) 
  { 
    try
    {
      m_set.symmetric_difference( cast(aOther).m_set); 
    } 
    catch(...)
    {
      show_error("Exception thrown during boolean operation");
    } 
  }
  
  static Self const& cast( Rep_base const& aOther ) { return dynamic_cast<Self const&>(aOther); }
  static Self      & cast( Rep_base      & aOther ) { return dynamic_cast<Self      &>(aOther); }
  
private:

  //For maintaining all drawing operations 
  GI* m_GI;
  //Storage for all polygons of one type. It is used as a base to perform all boolean operations
  Set m_set ;
  Gps_traits m_traits;
} ;

//A class for connecting GUI and this file
class Circular_rep : public Rep<Circular_GI, Circular_polygon_set,Circular_traits>
{
  typedef Rep<Circular_GI, Circular_polygon_set,Circular_traits> Base ;
  
public:
  
  Circular_rep () : Base() {} 
  
  virtual int type() const { return 2 ; }
} ;

//A class for connecting GUI and this file
class Linear_rep : public Rep<Linear_GI, Linear_polygon_set,Linear_traits>
{

typedef Rep<Linear_GI, Linear_polygon_set,Linear_traits> Base ;
public:
  
  Linear_rep () : Base() {}
  
  virtual int type() const { return 1 ; }
} ;

class Curve_set
{
  //a conatiner which deletes an object when last shared_ptr gets deleted or re-initiated
  typedef boost::shared_ptr<Rep_base> Rep_ptr ;
  
public:
  //constructor
  Curve_set( int aType, QPen aPen, QBrush aBrush ) : m_pen(aPen), m_brush(aBrush)
  {
    reset_type(aType);
  }
  void reset_type( int aType ) 
  {
    cout<<aType<<endl;
    //setting shared_ptr for repective polygon
    if(aType==1)
      m_rep = Rep_ptr(new Linear_rep());
    else
      m_rep=Rep_ptr(new Circular_rep());
    //setting pen and brush
    m_rep->set_pen  (m_pen);
    m_rep->set_brush(m_brush);
  }
  
  CGAL::Qt::GraphicsItem const* gi() const { return m_rep->gi() ; }
  CGAL::Qt::GraphicsItem*       gi()       { return m_rep->gi() ; }
  
  QRectF bounding_rect() const { return m_rep->bounding_rect() ; }
  
  bool is_empty() const { return !m_rep || m_rep->is_empty(); }
  
  void clear      () { m_rep->clear() ; }
  //boolean operations
  void complement () { m_rep->complement() ; }
  
  void assign ( Curve_set const& aOther ) 
  {
    return;
    if ( is_circular() && aOther.is_circular() )
    {
      get_circular_rep()->assign( *aOther.get_circular_rep() ) ;
    }
    else
    {
      get_linear_rep()->assign( *aOther.get_linear_rep() ) ;
    }
  }
  
  void intersect( Curve_set const& aOther ) 
  {
    return;
    
    if ( is_circular() && aOther.is_circular() )
    {
      get_circular_rep()->intersect( *aOther.get_circular_rep() ) ;
    }
    else
    {
      get_linear_rep()->intersect( *aOther.get_linear_rep() ) ;
    }
  }
  
  void join ( Curve_set const& aOther ) 
  {
    return;
      
    if ( is_circular() && aOther.is_circular() )
    {
      get_circular_rep()->join( *aOther.get_circular_rep() ) ;
    }
    else
    {
      get_linear_rep()->join( *aOther.get_linear_rep() ) ;
    }
  }
  
  void difference( Curve_set const& aOther ) 
  {
    return;
      
    if ( is_circular() && aOther.is_circular() )
    {
      get_circular_rep()->difference( *aOther.get_circular_rep() ) ;
    }
    else
    {
      get_linear_rep()->difference( *aOther.get_linear_rep() ) ;
    }
  }
  
  void symmetric_difference( Curve_set const& aOther ) 
  {
    return;
     
    if ( is_circular() && aOther.is_circular() )
    {
      get_circular_rep()->symmetric_difference( *aOther.get_circular_rep() ) ;
    }
    else
    {
      get_linear_rep()->symmetric_difference( *aOther.get_linear_rep() ) ;
    } 
  }
  //see its need keep it for now
  Rep_base const& rep() const { return *m_rep ; }
  Rep_base&       rep()       { return *m_rep ; }
  
  bool is_circular() const { return m_rep->type() == 2 ; }  
  bool is_linear  () const { return m_rep->type() == 1 ; } 
  
  //to get rep for circualr polygons
  Circular_rep const* get_circular_rep() const { cout<<"get const circular_rep"<<endl;return dynamic_cast<Circular_rep const*>( boost::get_pointer(m_rep) ); }
  Circular_rep      * get_circular_rep()       { cout<<"get normal circular_rep"<<endl;return dynamic_cast<Circular_rep*  >( boost::get_pointer(m_rep) ); }
  
  //to get Circular_polygon_set
  Circular_polygon_set const& circular() const { return get_circular_rep()->set(); }
  Circular_polygon_set      & circular()       { cout<<"get normal circular_polygon_set"<<endl;return get_circular_rep()->set(); }
  
  //to get rep for linear polygons
  Linear_rep const* get_linear_rep() const { cout<<"get const linear_rep"<<endl;return 
    dynamic_cast<Linear_rep const*>( boost::get_pointer(m_rep) ); }
  Linear_rep      * get_linear_rep()       { cout<<"get normal linear_rep"<<endl; return dynamic_cast<Linear_rep*  >( boost::get_pointer(m_rep) ); }
  
  //to get Linear_polygon_set
  Linear_polygon_set const& linear() const { return get_linear_rep()->set(); }
  Linear_polygon_set      & linear()       { cout<<"get normal linear_polygon_set"<<endl;return get_linear_rep()->set(); }
  
private:

  //drawing tools
  QPen                        m_pen ;
  QBrush                      m_brush ;
  //a conatiner which deletes an object when last shared_ptr gets deleted or re-initiated
  boost::shared_ptr<Rep_base> m_rep ;
  
} ;

typedef std::vector<Curve_set> Curve_set_container ;

typedef Curve_set_container::const_iterator Curve_set_const_iterator ;
typedef Curve_set_container::iterator       Curve_set_iterator ;


class MainWindow : public CGAL::Qt::DemosMainWindow ,
                   public Ui::Boolean_set_operations_2
{
  Q_OBJECT// removing it gives error ui not declared
  
private:  

  QGraphicsScene        m_scene;
  //keep it intact for now check it out
  bool                  m_circular_active ;
  bool                  m_linear_active ;
  //which type is currently active now
  bool                  m_blue_active ;
  Curve_set_container   m_curve_sets ;
  //container for curves
  Circular_region_source_container   m_blue_circular_sources ;
  Circular_region_source_container   m_red_circular_sources ;
  Linear_region_source_container     m_blue_linear_sources ; 
  Linear_region_source_container     m_red_linear_sources ; 
  
  //typedefs of classes used to draw circular and linear polygon
  CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>*     m_linear_input ;
  CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>*   m_circular_input ;
   
public:

  MainWindow();

private:
  
  void dragEnterEvent(QDragEnterEvent *event);
  void dropEvent(QDropEvent *event);
  void zoomToFit();
  //1->linear polygons   2->circular polygons
  int m_polygon_type=2;
  
protected slots:
  
  void open( QString filename ) ;

public slots:
  
  void processInput(CGAL::Object o);
  void on_actionNew_triggered();
  void on_actionRecenter_triggered();

  void on_actionInsertLinear_triggered();//bool aCheck);
  void on_actionInsertCircular_triggered();//bool aCheck);
    
signals:
   //see if the demo runs without it
  void modelChanged();
  
private:
  /*
  void Changed()
  {
    emit(modelChanged());
  }
  */
  
  //warning message for user
  bool ask_user_yesno( const char* aTitle, const char* aQuestion )
  {
    return QMessageBox::warning(this
                               ,aTitle
                               ,QString(aQuestion)
                               ,"&Yes"
                               ,"&No"
                               ,QString::null
                               , 1
                               , 1 
                               ) == 0 ;
  }
  
  //for setting Curve_set of aGroup type an int representing a set of polygon of a specific type
  Curve_set& set( int aGroup ) { cout<<"set function"<<endl;
  return m_curve_sets[aGroup] ; }
  
  //setting curve
  Curve_set& blue_set  () { return set(BLUE_GROUP)  ; }
  Curve_set& red_set   () { return set(RED_GROUP)   ; }
  Curve_set& result_set() { return set(RESULT_GROUP); }

  //gets which group is currently active now
  int active_group() const { return m_blue_active ? BLUE_GROUP : RED_GROUP ; }
  
  //sets the current active group
  Curve_set& active_set()   { return set(active_group()) ; }

  //returns circular containers
  Circular_region_source_container const& blue_circular_sources() const { return m_blue_circular_sources ; }
  Circular_region_source_container      & blue_circular_sources()       { return m_blue_circular_sources ; }

  Circular_region_source_container const& red_circular_sources () const { return m_red_circular_sources ; }
  Circular_region_source_container      & red_circular_sources ()       { return m_red_circular_sources ; }
  
  //returns linear containers
  Linear_region_source_container const& blue_linear_sources() const { return m_blue_linear_sources ; }
  Linear_region_source_container      & blue_linear_sources()       { return m_blue_linear_sources ; }

  Linear_region_source_container const& red_linear_sources () const { return m_red_linear_sources ; }
  Linear_region_source_container      & red_linear_sources ()       { return m_red_linear_sources ; }

  //returns active blue container
  Circular_region_source_container const& active_circular_sources() const { return m_blue_active ? m_blue_circular_sources : m_red_circular_sources ; }
  Circular_region_source_container      & active_circular_sources()       { return m_blue_active ? m_blue_circular_sources : m_red_circular_sources ; }

  //returns active linear container
  Linear_region_source_container const& active_linear_sources() const { return m_blue_active ? m_blue_linear_sources : m_red_linear_sources ; }
  Linear_region_source_container      & active_linear_sources()       { return m_blue_active ? m_blue_linear_sources : m_red_linear_sources ; }  
  
  //changes the set of polygons of a specific type
  void ToogleView( int aGROUP, bool aChecked );
  
  void link_GI( CGAL::Qt::GraphicsItem* aGI )
  {
    QObject::connect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
    m_scene.addItem( aGI );
  }
  
  void unlink_GI( CGAL::Qt::GraphicsItem* aGI )
  {
    m_scene.removeItem( aGI );
    QObject::disconnect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
  }
  
  void switch_set_type( Curve_set& aSet, int aType );
  
  void switch_sets_type( int aType );
  
  bool ensure_circular_mode();
  
  bool ensure_linear_mode();//see if it is need
};


MainWindow::MainWindow()
  : DemosMainWindow()
  , m_circular_active(true)//default
  , m_blue_active(true)    //default
{
  setupUi(this);

  setAcceptDrops(true);
  cout<<"elementry setups"<<endl;
  //default setups
  m_curve_sets.push_back( Curve_set(2, sPens[BLUE_GROUP]  , sBrushes[BLUE_GROUP]  ) ) ;
  m_curve_sets.push_back( Curve_set(2, sPens[RED_GROUP]   , sBrushes[RED_GROUP]   ) ) ;
  m_curve_sets.push_back( Curve_set(2, sPens[RESULT_GROUP], sBrushes[RESULT_GROUP]) ) ;
  
  //m_curve_sets.push_back( Curve_set(1, sPens[BLUE_GROUP]  , sBrushes[BLUE_GROUP]  ) ) ;
  //m_curve_sets.push_back( Curve_set(1, sPens[RED_GROUP]   , sBrushes[RED_GROUP]   ) ) ;
  //m_curve_sets.push_back( Curve_set(1, sPens[RESULT_GROUP], sBrushes[RESULT_GROUP]) ) ;
  cout<<"curve setups"<<endl;
  for( Curve_set_iterator si = m_curve_sets.begin(); si != m_curve_sets.end() ; ++ si )
  { cout<<"setting curves"<<endl;
    link_GI(si->gi()) ;
  }
  //
  // Setup the m_scene and the view
  //
  m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  m_scene.setSceneRect(-100, -100, 100, 100);
  this->graphicsView->setScene(&m_scene);
  this->graphicsView->setMouseTracking(true);

  // Turn the vertical axis upside down 
  this->graphicsView->scale(1, -1);
    cout<<"UI setup"<<endl;
    
  //adding basic setups
    
  // The navigation adds zooming and translation functionality to the
  // QGraphicsView
  this->addNavigation(this->graphicsView);
  //setting the menus
  this->setupStatusBar();
  this->setupOptionsMenu();
  //link to a page describing
  this->addAboutDemo(":/cgal/help/index.html");
  //link for about page of CGAL
  this->addAboutCGAL();

  this->addRecentFiles(this->menuFile, this->actionQuit);
  cout<<"extra setup"<<endl;
  
  //initializing classes to draw respective polygons using mouse
  m_linear_input  =new CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>(this, &m_scene);
  m_circular_input=new CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>(this, &m_scene);
    
  //connecting GUI and the code base
  QObject::connect(m_linear_input  , SIGNAL(generate(CGAL::Object)), this, SLOT(processInput(CGAL::Object)));
  QObject::connect(m_circular_input, SIGNAL(generate(CGAL::Object)), this, SLOT(processInput(CGAL::Object)));
  
  QObject::connect(this->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
  //QObject::connect(this->actionInsertCircular, SIGNAL(triggered()), this, SLOT(on_actionInsertCircular_triggered()));
  //QObject::connect(this->actionInsertLinear, SIGNAL(triggered()), this, SLOT(on_actionInsertLinear_triggered()));
  //QObject::connect(this, SIGNAL(openRecentFile(QString)), this, SLOT(open(QString)));//for file handling
  cout<<"connecting stuff"<<endl;
}

//keep it no use for now
void MainWindow::on_actionNew_triggered() 
{
  for( Curve_set_iterator si = m_curve_sets.begin(); si != m_curve_sets.end() ; ++ si )
    si->clear();
 cout<<"In new Polygon"<<endl;
 blue_circular_sources().clear();
 blue_linear_sources().clear();
    
  ToogleView(BLUE_GROUP  ,true);
  m_circular_active = true ;
  m_blue_active =  true ;
  //modelChanged();
}

//extra utilities
void MainWindow::on_actionRecenter_triggered()
{
  zoomToFit();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list"))
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
  QString filename = event->mimeData()->urls().at(0).path();
  open(filename);
  event->acceptProposedAction();
}

//for converting linear part of circular polygon to circular part
Circular_polygon linear_2_circ( Circular_Linear_polygon const& pgn )
{
  CGAL::Cartesian_converter<Kernel,Kernel> convert ;
  
  Circular_polygon rCP;
  
  for( Circular_Linear_polygon::Edge_const_iterator ei = pgn.edges_begin(); ei != pgn.edges_end(); ++ei )
  {
    if  ( ei->source() != ei->target() )
      rCP.push_back( Circular_X_monotone_curve( convert(ei->source()), convert(ei->target())) );
  }  

  return rCP;
}

//for converting linear part of circular polygon with holes to circular part
Circular_polygon_with_holes linear_2_circ( Circular_Linear_polygon_with_holes const& pwh )
{
  Circular_polygon_with_holes rCP( linear_2_circ(pwh.outer_boundary()) ) ;
  
  for( Circular_Linear_polygon_with_holes::Hole_const_iterator hi = pwh.holes_begin(); hi != pwh.holes_end(); ++ hi )
    rCP.add_hole( linear_2_circ(*hi)  );

  return rCP;
}

//check out
void MainWindow::switch_set_type( Curve_set& aSet, int aType )
{
  unlink_GI( aSet.gi() ) ;
  
  aSet.reset_type(aType);
  
  link_GI( aSet.gi() ) ;
  
  //modelChanged();
}

void MainWindow::switch_sets_type( int aType )
{
  switch_set_type( blue_set  (), aType ) ; 
  switch_set_type( red_set   (), aType ) ; 
  switch_set_type( result_set(), aType ) ; 
}

bool MainWindow::ensure_circular_mode()
{
  
  if ( ! m_circular_active )
  {
    bool lProceed = blue_set().is_empty() && red_set().is_empty() ;
    
    if ( ! lProceed )
      lProceed = ask_user_yesno("Linear/Circular mode switch"
                               ,"You are about to load a linear poygon, but there are circular curves already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the circular curves must be removed first.\n" \
                                "Yes to remove and proceed?\n"
                               ) ;
      
    if ( lProceed )
    {
      switch_sets_type(2);
      m_circular_active = true ;
    }
  }
  return m_circular_active ;
}

bool MainWindow::ensure_linear_mode()
{
 
  if ( m_circular_active )
  {
    bool lProceed = blue_set().is_empty() && red_set().is_empty() ;
    
    if ( ! lProceed )
      lProceed = ask_user_yesno("Linear/Circular mode switch"
                               ,"You are about to load a circular poygon, but there are linear curves already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the linear curves must be removed first.\n" \
                                "Yes to remove and proceed?\n"
                               ) ;
      
    if ( lProceed )
    {
      switch_sets_type(1);
      m_circular_active = false ;
    }
  }
  return !m_circular_active ;
}
//check out

void MainWindow::open( QString fileName )
{
  cout<<"To be done"<<endl;
    if(! fileName.isEmpty())
  {
    bool lRead = false ;
     
    if ( lRead )
    {
      //modelChanged();
      zoomToFit();
      this->addToRecentFiles(fileName);
      
    }
  }  
}

void MainWindow::on_actionInsertCircular_triggered()
{
  //bool aCheck=1;
  cout<<"signal circular triggered"<<endl;
  m_scene.installEventFilter(m_circular_input);    
}

void MainWindow::on_actionInsertLinear_triggered()
{
  cout<<"signal linear triggered"<<endl;
  m_scene.installEventFilter(m_linear_input);
}

void MainWindow::processInput(CGAL::Object o )
{
  m_blue_active =  true ;
  
  Linear_polygon   lLI ;
  Circular_polygon lCI ;
     
  cout<<"process input"<<endl;  
  if(CGAL::assign(lLI, o))
  {
    cout<<"came to linear"<<endl;
    if ( ensure_linear_mode() )
    {
      cout<<"inside linear"<<endl;
      CGAL::Orientation o = lLI.orientation();
      //return;
      //cout<<"set linear's orientation"<<endl;
      if( o == CGAL::CLOCKWISE )
      {
        cout<<"passed if"<<endl;
        lLI.reverse_orientation();
      }
      cout<<"oriented"<<endl;
      Linear_polygon_with_holes lCPWH(lLI);
      cout<<"l l l l"<<endl;
      active_set().linear().join(lCPWH) ;  
      cout<<"hi linear"<<endl;
      active_linear_sources().push_back(lCPWH);
      cout<<"processed linear"<<endl;
    }
  }

  else if ( CGAL::assign(lCI, o) )
  {
    cout<<"came to circular"<<endl;
    if ( ensure_circular_mode() )
    {
      cout<<"inside circular"<<endl;
      CGAL::Orientation o = lCI.orientation();
      //cout<<"set circular's orientation"<<endl;
      if ( o == CGAL::CLOCKWISE )
        lCI.reverse_orientation();

      cout<<"oriented"<<endl;
      Circular_polygon_with_holes lCPWH(lCI);
      cout<<"c c c c"<<endl;
      active_set().circular().join(lCPWH) ;  
      cout<<"hi circular"<<endl;
      active_circular_sources().push_back(lCPWH);
      cout<<"processed circualar"<<endl;
    }
  }
  //modelChanged();  
    
}

void MainWindow::ToogleView( int aGROUP, bool aChecked )
{
  if ( aChecked )
    set(aGROUP).gi()->show();
  else 
    set(aGROUP).gi()->hide();
}


void MainWindow::zoomToFit()
{
  boost::optional<QRectF> lTotalRect ;
  
  for ( Curve_set_const_iterator si = m_curve_sets.begin() ; si != m_curve_sets.end() ; ++ si )
  {
    if ( !si->is_empty() ) 
    {
      QRectF lRect = si->bounding_rect();
      if ( lTotalRect )
           lTotalRect = *lTotalRect | lRect ;
      else lTotalRect = lRect ;  
    }
  }
                   
  if ( lTotalRect )
  {
    this->graphicsView->setSceneRect(*lTotalRect);
    this->graphicsView->fitInView(*lTotalRect, Qt::KeepAspectRatio);  
  }                 
}

//Main part
#include "Boolean_set_operations_2.moc"
#include <CGAL/Qt/resources.h>
int main(int argc, char *argv[])
{
  //QApplication a(argc, argv);
  QApplication app(argc, argv);

  app.setOrganizationDomain("geometryfactory.com");
  app.setOrganizationName("GeometryFactory");
  app.setApplicationName("Boolean_operations_2 demo");
  CGAL_QT_INIT_RESOURCES;
  try
  {
//std::cout<<"hello";    
    MainWindow w;
    w.show();

    return app.exec();
  }
  catch (const std::exception e)
  {
    std::string s = e.what();
    show_error("Exception throne during run of the program:\n" + s);
  }
}
