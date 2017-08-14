// Copyright (c) 2008, 2012  GeometryFactory Sarl (France).
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
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef CGAL_QT_ARRANGEMENT_GRAPHICS_ITEM_H
#define CGAL_QT_ARRANGEMENT_GRAPHICS_ITEM_H

#include <CGAL/Bbox_2.h>
//#include <CGAL/apply_to_range.h>
// TODO: should be included in PainterOstream.h
//#include <CGAL/Kernel/global_functions.h>
#include <CGAL/Qt/GraphicsItem.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Arr_circular_arc_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>

#include <QGraphicsScene>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
//#include <QStyleOption>

#include "ArrangementPainterOstream.h"
#include "Utils.h"

#include <iostream>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include "ui_ArrangementDemoWindow.h"
#include "ArrangementDemoGraphicsView.h"

class QGraphicsScene;
class QApplication;

extern Ui::ArrangementDemoWindow* getCurrentDemoWindowUi();
extern ArrangementDemoGraphicsView* getCurrentView();

namespace CGAL {
namespace Qt {

class ArrangementGraphicsItemBase :
    public GraphicsItem, public QGraphicsSceneMixin
{
public:
  ArrangementGraphicsItemBase( ):
    bb( 0, 0, 0, 0 ),
    bb_initialized( false ),
    verticesPen( QPen( ::Qt::blue, 3. ) ),
    edgesPen( QPen( ::Qt::blue, 1. ) ),
    visible_edges( true ),
    visible_vertices( true ),
    scene( NULL ),
    backgroundColor( ::Qt::white ),
    scalingFactor(1.0)
  {
    this->verticesPen.setCosmetic( true );
    this->verticesPen.setCapStyle( ::Qt::SquareCap );
    this->edgesPen.setCosmetic( true );
    this->firstEntry = true;
  }

  const QPen& getVerticesPen( ) const
  {
    return this->verticesPen;
  }

  const QPen& getEdgesPen( ) const
  {
    return this->edgesPen;
  }

  void setVerticesPen( const QPen& pen )
  {
    this->verticesPen = pen;
  }

  void setEdgesPen( const QPen& pen )
  {
    this->edgesPen = pen;
  }

  bool visibleVertices( ) const
  {
    return this->visible_vertices;
  }

  void setVisibleVertices( const bool b )
  {
    this->visible_vertices = b;
    this->update( );
  }

  bool visibleEdges( ) const
  {
    return this->visible_edges;
  }

  void setVisibleEdges( const bool b )
  {
    this->visible_edges = b;
    this->update( );
  }

  void setBackgroundColor( QColor color )
  {
    this->backgroundColor = color;
  }

  void setScene( QGraphicsScene* scene_ )
  {
    this->QGraphicsSceneMixin::setScene( scene_ );
    this->scene = scene_;
  }

protected:

  QRectF viewportRect( ) const
  {
    std::cout<<"In ArrangementGraphicsItemBase viewportRect\n";
    QRectF res;
    if ( this->scene == NULL )
    {
      std::cout<<"Return: this->scene == NULL\n";
      return res;
    }

    QList< QGraphicsView* > views = this->scene->views( );
    if ( views.size( ) == 0 )
    {
      std::cout<<"Return: views.size( ) == 0\n";
      return res;
    }
    // assumes the first view is the right one
    QGraphicsView* viewport = views.first( );
    QPointF p1 = viewport->mapToScene( 0, 0 );
    QPointF p2 = viewport->mapToScene(viewport->width(), viewport->height());

    double xmin = (std::min)(p1.x(), p2.x());
    double xmax = (std::max)(p1.x(), p2.x());
    double ymin = (std::min)(p1.y(), p2.y());
    double ymax = (std::max)(p1.y(), p2.y());

    res = QRectF( QPointF( xmin, ymin ), QPointF( xmax, ymax ) );

    std::cout<<"Return: OKAY\n";
    return res;
  }

  CGAL::Bbox_2 bb;
  bool bb_initialized;

  QPen verticesPen;
  QPen edgesPen;
  bool visible_edges;
  bool visible_vertices;

  QGraphicsScene* scene;

  QColor backgroundColor;
  double scalingFactor;
  bool firstEntry;

}; // class ArrangementGraphicsItemBase


template <typename Arr_, typename ArrTraits = typename Arr_::Geometry_traits_2>
class ArrangementGraphicsItem : public ArrangementGraphicsItemBase
{
  typedef Arr_ Arrangement;
  typedef typename Arrangement::Geometry_traits_2       Traits;
  typedef typename Arrangement::Vertex_iterator         Vertex_iterator;
  typedef typename Arrangement::Curve_iterator          Curve_iterator;
  typedef typename Arrangement::Edge_iterator           Edge_iterator;
  typedef typename Arrangement::Halfedge                Halfedge;
  typedef typename Arrangement::Halfedge_handle         Halfedge_handle;
  typedef typename Arrangement::Face_handle             Face_handle;
  typedef typename Arrangement::Face_iterator           Face_iterator;
  typedef typename Arrangement::Unbounded_face_iterator Unbounded_face_iterator;
  typedef typename Arrangement::Hole_iterator           Holes_iterator;
  typedef typename Arrangement::Ccb_halfedge_circulator Ccb_halfedge_circulator;

  typedef typename ArrTraitsAdaptor< Traits >::Kernel   Kernel;
  typedef typename Traits::X_monotone_curve_2           X_monotone_curve_2;
  typedef typename Kernel::Point_2                      Kernel_point_2;
  typedef typename Traits::Point_2                      Point_2;
  //typedef typename Kernel::Segment_2 Segment_2;

  typedef ArrangementGraphicsItemBase                   Superclass;
  typedef typename Kernel::Segment_2                    Segment_2;
  typedef typename Traits::Curve_2                      Curve_2;

public:
  /*! Constructor */
  ArrangementGraphicsItem( Arrangement* t_ );

  /*! Destructor (virtual) */
  ~ArrangementGraphicsItem() {}

public:
  void modelChanged( );
  QRectF boundingRect( ) const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                     QWidget* widget);

protected:

  template < typename TTraits >
  void modelChanged(TTraits traits);

  template < typename Coefficient_>
  void modelChanged(CGAL::Arr_algebraic_segment_traits_2<Coefficient_>
                         traits);

  template < typename TTraits >
  void paint( QPainter* painter, TTraits traits );

  template < typename CircularKernel >
  void paint( QPainter* painter,
              CGAL::Arr_circular_arc_traits_2< CircularKernel > traits );

  template < typename Coefficient_ >
  void paint( QPainter* painter,
              CGAL::Arr_algebraic_segment_traits_2< Coefficient_ > traits );

  void updateBoundingBox();

  template < typename TTraits>
  void updateBoundingBox(TTraits traits );

  template < typename Kernel_>
  void updateBoundingBox(CGAL::Arr_linear_traits_2<Kernel_> traits);

  template < typename Coefficient_>
  void updateBoundingBox(CGAL::Arr_algebraic_segment_traits_2<Coefficient_>
                         traits);

  Arrangement* arr;
  ArrangementPainterOstream< Traits > painterostream;
  CGAL::Qt::Converter< Kernel > convert;
  std::map< Curve_iterator, CGAL::Bbox_2 > curveBboxMap;


  void paintFaces( QPainter* painter )
  {
    typename Traits::Left_side_category category;
    this->paintFaces( painter, category );
  }

  void paintFaces( QPainter* painter, CGAL::Arr_oblivious_side_tag )
  {
    // Prepare all faces for painting
    std::cout<<"In paintFaces Arr_oblivious_side_tag"<<std::endl;
    int Face_iterator_cnt = 0;
    for( Face_iterator fi = this->arr->faces_begin( );
         fi != this->arr->faces_end( ); ++fi )
    {
      fi->set_visited( false );
      if ( fi->is_unbounded( ) )
      {
        std::cout << "setting unbounded face not visited" << std::endl;
      }
      else
      {
        std::cout << "setting bounded face not visited" << std::endl;
      }
      Face_iterator_cnt++;
    }

    std::cout<<"First Face_iterator cnt: "<<Face_iterator_cnt <<std::endl;
    Face_iterator_cnt = 0;

    for( Face_iterator fi = this->arr->faces_begin( );
         fi != this->arr->faces_end( ); ++fi )
    {
      Face_iterator_cnt++;
      Face_handle f_handle = fi;
      this->paintFace( f_handle, painter );
    }

    std::cout<<"Second Face_iterator cnt: "<<Face_iterator_cnt <<std::endl;
    
    std::cout<<"Leaving paintFaces Arr_oblivious_side_tag"<<std::endl;
  }

  void paintFaces( QPainter* painter, CGAL::Arr_open_side_tag )
  {


    // Face_handle fictitiousFace = this->arr->fictitious_face( );
    std::cout<<"In paintFaces Arr_open_side_tag"<<std::endl;
    std::cout<<"number_of_unbounded_faces: ";
    std::cout<<this->arr->number_of_unbounded_faces()<<std::endl;

    // Prepare all faces for painting
    int Face_iterator_cnt = 0;

    for( Face_iterator fi = this->arr->faces_begin( );
         fi != this->arr->faces_end( ); ++fi )
    {

      if ( fi->is_fictitious( ) )
      {
          std::cout << "setting fictitious face not visited" << std::endl;
      }
      if ( fi->is_unbounded( ) )
      {
          std::cout << "setting unbounded face not visited" << std::endl;
      }
      Face_iterator_cnt++;
      fi->set_visited( false );
    }

    std::cout<<"First Face_iterator cnt: "<<Face_iterator_cnt <<std::endl;
    Face_iterator_cnt = 0;

    for( Face_iterator fi = this->arr->faces_begin( );
         fi != this->arr->faces_end( ); ++fi )
    {
      Face_iterator_cnt++;
      Face_handle f_handle = fi;
      this->paintFace( f_handle, painter );
    }

    std::cout<<"Second Face_iterator cnt: "<<Face_iterator_cnt <<std::endl;
    // Face_handle unboundedFace = this->arr->unbounded_face( );
    // this->paintFace( unboundedFace, painter );
    
    std::cout<<"Leaving paintFaces Arr_open_side_tag"<<std::endl;
  }

#if 0
  template < typename TTraits >
  void paintFaces( QPainter* painter, TTraits traits ) { }

  template < typename Kernel_ >
  void paintFaces(QPainter* painter, CGAL::Arr_segment_traits_2<Kernel_>) {}

  template < typename Kernel_ >
  void paintFaces(QPainter* painter, CGAL::Arr_polyline_traits_2<Kernel_>) {}

  template < typename RatKernel, class AlgKernel, class NtTraits >
  void paintFaces(QPainter* painter,
                  CGAL::Arr_conic_traits_2<RatKernel, AlgKernel, NtTraits >)
  {}

  template < typename CircularKernel >
  void paintFaces(QPainter* painter,
                  CGAL::Arr_circular_arc_traits_2<CircularKernel > traits) {}

  template < typename Coefficient_ >
  void paintFaces(QPainter* painter,
                  CGAL::Arr_algebraic_segment_traits_2<Coefficient_> traits) {}
#endif

  void paintFace( Face_handle f, QPainter* painter );

  void visit_ccb_faces( Face_handle & fh, QPainter* painter )
  {
    this->paintFace( fh, painter );

    Ccb_halfedge_circulator cc=fh->outer_ccb();
    do {
      Halfedge he = *cc;
      if (! he.twin()->face()->visited())
      {
        Face_handle nei = (Face_handle) he.twin()->face();
        this->visit_ccb_faces( nei , painter );
      }
      //created from the outer boundary of the face
    } while (++cc != fh->outer_ccb());
  }

  /*! antenna - return true if the halfedge and its
   *  twin point to the same face.
   */
  bool antenna(Halfedge_handle h)
  {
    Halfedge_handle twin = h->twin();
    return (twin->face() == h->face());
  }

  template < typename Traits >
  void paintFace(Face_handle /* f */, QPainter* /* painter */,
                 Traits /* traits */)
  {
      std::cout<<"In paintFace Traits"<<std::endl;
  }

#if 0
  template < typename Coefficient_ >
  void paintFace( Face_handle f, QPainter* painter,
                  CGAL::Arr_algebraic_segment_traits_2< Coefficient_ > )
  {
    typedef CGAL::Arr_algebraic_segment_traits_2<Coefficient> Traits;
    typedef typename ArrTraitsAdaptor<Traits>::Kernel     Kernel;
    typedef Kernel::Point_2                               Kernel_point_2;
    Arr_construct_point_2< Traits > toArrPoint;


    std::cout<<"In paintFace Arr_algebraic_segment_traits_2"<<std::endl;

    if ( f->is_unbounded() )
    {
      std::cout<<"In paintFace Arr_algebraic_segment_traits_2 unbounded"<<std::endl;
      QRectF rect = this->viewportRect( );
      QColor color = this->backgroundColor;
      painter->fillRect( rect, color );
      std::cout<<"Leaving paintFace Arr_algebraic_segment_traits_2 unbounded"<<std::endl;
      return;
    }

    std::cout<<"In paintFace Arr_algebraic_segment_traits_2 bounded"<<std::endl;
    QVector< QPointF > pts; // holds the points of the polygon

    /* running with around the outer of the face and generate from it
     * polygon
     */
    Ccb_halfedge_circulator cc=f->outer_ccb();
    do {
      Halfedge_handle he = cc;
      X_monotone_curve_2 c = he->curve();
      // Get the co-ordinates of the curve's source and target.
      double sx = CGAL::to_double(he->source()->point().x()),
        sy = CGAL::to_double(he->source()->point().y()),
        tx = CGAL::to_double(he->target()->point().x()),
        ty = CGAL::to_double(he->target()->point().y());

      QPointF coord_source(sx, sy);
      QPointF coord_target(tx, ty);

      // Transform the point coordinates from general coordinate system to
      // Qt scene coordinate system
      QPoint coord_source_viewport = this->fromScene( coord_source );
      QPoint coord_target_viewport = this->fromScene( coord_target );

      // If the curve is monotone, than its source and its target has the
      // extreme x co-ordinates on this curve.
      bool is_source_left = (sx < tx);
      int  x_min = is_source_left ? coord_source_viewport.x( ) : coord_target_viewport.x( );
      int  x_max = is_source_left ? coord_target_viewport.x( ) : coord_source_viewport.x( );

      std::cout<<"x_min\t"<<x_min<<std::endl;
      std::cout<<"x_max\t"<<x_max<<std::endl;
      // double curr_x, curr_y;

      // Alg_seg_point_2 px;

      // pts.push_back(coord_source );

      // // Draw the curve as pieces of small segments
      // const int DRAW_FACTOR = 5;
      // for ( int x = x_min + DRAW_FACTOR; x < x_max; x+=DRAW_FACTOR )
      // {
      //   //= COORD_SCALE)
      //   curr_x = this->toScene( x );
      //   Alg_kernel   ker;
      //   Kernel_point_2 ker_point_2(curr_x, 0);
      //   Alg_seg_point_2 curr_p = toArrPoint(ker_point_2);

      //   // If curr_x > x_max or curr_x < x_min
      //   if (!(ker.compare_x_2_object()(curr_p, c.left()) !=
      //         CGAL::SMALLER &&
      //         ker.compare_x_2_object()(curr_p, c.right()) !=
      //         CGAL::LARGER))
      //   {
      //     continue;
      //   }

      //   px = c.point_at_x (curr_p);
      //   curr_y = CGAL::to_double(px.y());
      //   QPointF curr( curr_x, curr_y );
      //   pts.push_back( curr );
      // }// for

      // pts.push_back(coord_target );

    } while (++cc != f->outer_ccb());

    // make polygon from the outer ccb of the face 'f'
    QPolygonF pgn (pts);

    // FIXME: get the bg color
    QColor color = this->backgroundColor;
    if ( f->color().isValid() )
    {
      color = f->color();
    }

    QBrush oldBrush = painter->brush( );
    painter->setBrush( color );
    painter->drawPolygon( pgn );
    painter->setBrush( oldBrush );

    std::cout<<"Leaving paintFace Arr_algebraic_segment_traits_2 bounded"<<std::endl;
  }
#endif
  template < typename Kernel_ >
  void paintFace( Face_handle f, QPainter* painter,
                  CGAL::Arr_segment_traits_2< Kernel_ > );

  template < typename Kernel_ >
  void paintFace( Face_handle f, QPainter* painter,
                  CGAL::Arr_polyline_traits_2< Kernel_ > );


#ifdef CGAL_USE_CORE
  template < typename RatKernel, typename AlgKernel, typename NtTraits >
  void 
  paintFace( Face_handle f, QPainter* painter,
                  CGAL::Arr_conic_traits_2< RatKernel, AlgKernel, NtTraits > )
  {
    std::cout<<"In paintFace Arr_conic_traits_2"<<std::endl;

    if (! f->is_unbounded())  // f is not the unbounded face
    {
      std::cout<<"In paintFace Arr_conic_traits_2 bounded"<<std::endl;
      QVector< QPointF > pts; // holds the points of the polygon
      /* running with around the outer of the face and generate from it
       * polygon
       */
      Ccb_halfedge_circulator cc=f->outer_ccb();
      do
      {
        if (this->antenna(cc))
        {
          continue;
        }

        Halfedge_handle he = cc;
        X_monotone_curve_2 c = he->curve();
        // Get the co-ordinates of the curve's source and target.
        double sx = CGAL::to_double(he->source()->point().x()),
          sy = CGAL::to_double(he->source()->point().y()),
          tx = CGAL::to_double(he->target()->point().x()),
          ty = CGAL::to_double(he->target()->point().y());

        QPointF coord_source(sx, sy);
        QPointF coord_target(tx, ty);

        // Transform the point coordinates from general coordinate system to
        // Qt scene coordinate system
        QPoint coord_source_viewport = this->fromScene( coord_source );
        QPoint coord_target_viewport = this->fromScene( coord_target );

        if (c.orientation() == CGAL::COLLINEAR)
        {
          pts.push_back(coord_source );
        }
        else
        {
          // If the curve is monotone, than its source and its target has the
          // extreme x co-ordinates on this curve.
          bool is_source_left = (sx < tx);
          int  x_min = is_source_left ?
            coord_source_viewport.x( ) : coord_target_viewport.x( );
          int  x_max = is_source_left ?
            coord_target_viewport.x( ) : coord_source_viewport.x( );
          double curr_x, curr_y;
          int x;

          Arr_conic_point_2 px;

          pts.push_back(coord_source );

          // Draw the curve as pieces of small segments
          const int DRAW_FACTOR = 5;
          if (is_source_left)
          {
            for ( x = x_min + DRAW_FACTOR; x < x_max; x+=DRAW_FACTOR )
            {
              //= COORD_SCALE)
              curr_x = this->toScene( x );
              Alg_kernel   ker;
              Arr_conic_point_2 curr_p(curr_x, 0);

              // If curr_x > x_max or curr_x < x_min
              if (!(ker.compare_x_2_object()(curr_p, c.left()) !=
                    CGAL::SMALLER &&
                    ker.compare_x_2_object()(curr_p, c.right()) !=
                    CGAL::LARGER))
              {
                continue;
              }

              px = c.point_at_x (curr_p);
              curr_y = CGAL::to_double(px.y());
              QPointF curr( curr_x, curr_y );
              pts.push_back( curr );
            }// for
          }
          else
          {
            for ( x = x_max; x > x_min; x-=DRAW_FACTOR )
            {
              curr_x = this->toScene( x );
              Alg_kernel   ker;
              Arr_conic_point_2 curr_p(curr_x, 0);
              if (!(ker.compare_x_2_object() (curr_p, c.left()) !=
                    CGAL::SMALLER &&
                    ker.compare_x_2_object() (curr_p, c.right()) !=
                    CGAL::LARGER))
              {
                continue;
              }

              px = c.point_at_x (curr_p);
              curr_y = CGAL::to_double(px.y());
              QPointF curr( curr_x, curr_y );
              pts.push_back( curr );
            }// for
          }// else
          pts.push_back(coord_target );
        }
        //created from the outer boundary of the face
      } while (++cc != f->outer_ccb());

      // make polygon from the outer ccb of the face 'f'
      QPolygonF pgn( pts );

      // fill the face according to its color (stored at any of her
      // incidents curves)
      QBrush oldBrush = painter->brush( );
      QColor def_bg_color = this->backgroundColor;
      if (! f->color().isValid())
      {
        painter->setBrush( def_bg_color );
      }
      else
      {
        painter->setBrush( f->color( ) );
      }

      QPen pen = painter->pen();
      pen.setCosmetic(true);
      painter->setPen(pen);

      painter->drawPolygon( pgn );
      painter->setBrush( oldBrush );
    }
    else
    {
      std::cout<<"In paintFace Arr_conic_traits_2 unbounded"<<std::endl;
      QRectF rect = this->viewportRect( );
      std::cout<<rect.left()<<'\t';
      std::cout<<rect.right()<<'\t';
      std::cout<<rect.top()<<'\t';
      std::cout<<rect.bottom()<<'\n';

      QColor color = this->backgroundColor;
      if ( f->color().isValid() )
      {
        color = f->color();
      }
      QBrush oldBrush = painter->brush( );
      QPen pen = painter->pen();
      pen.setCosmetic(true);
      painter->setPen(pen);
      painter->setBrush( color );
      painter->drawRect(rect);
      painter->setBrush( oldBrush );

      std::cout<<"Leaving paintFace Arr_conic_traits_2 unbounded"<<std::endl;
    }
  }

#endif

  template < typename CircularKernel >
  void paintFace(Face_handle f, QPainter* painter,
                 CGAL::Arr_circular_arc_traits_2<CircularKernel> /* traits */);

  template < typename Coefficient_ >
  X_monotone_curve_2 makeSegment(double x1, double y1, double x2, double y2,
                        CGAL::Arr_algebraic_segment_traits_2<
                                   Coefficient_ > /* traits */)
  {
    typedef Coefficient_ Coefficient;
    typedef CGAL::Arr_algebraic_segment_traits_2< Coefficient > Traits;
    Traits traits;

    typedef typename Traits::X_monotone_curve_2           X_monotone_curve_2;
    typedef typename Traits::Curve_2           Curve_2;

    typename Traits::Construct_x_monotone_segment_2 constructSegment =
      traits.construct_x_monotone_segment_2_object( );

    Point_2 p1(x1, y1);
    Point_2 p2(x2, y2);
    std::vector< X_monotone_curve_2 > curves;

    constructSegment( p1, p2, std::back_inserter( curves ) );
    Curve_2 curve = curves[0].curve();

    std::cout<<"In makeSegment, curves size: "<<curves.size()<<std::endl;

    return curves[0];
  }

  template < typename Coefficient_ >
  void paintFace(Face_handle f, QPainter* painter,
                 CGAL::Arr_algebraic_segment_traits_2<
                                   Coefficient_ > /* traits */)
 {
    typedef Coefficient_ Coefficient;
    typedef CGAL::Arr_algebraic_segment_traits_2< Coefficient > Traits;
    typedef typename Traits::X_monotone_curve_2           X_monotone_curve_2;
    typedef typename Traits::Polynomial_2           Polynomial_2;

    Traits traits;
    typename Traits::Construct_curve_2 construct_curve
      = traits.construct_curve_2_object();

    std::cout<<"In paintFace Arr_algebraic_segment_traits_2"<<std::endl;
    if (f->is_unbounded())
    {

      std::cout<<"In paintFace Arr_algebraic_segment_traits_2 unbounded"<<std::endl;
#if 0
      QRectF rect = this->viewportRect( );
      std::cout<<rect.left()<<'\t';
      std::cout<<rect.right()<<'\t';
      std::cout<<rect.top()<<'\t';
      std::cout<<rect.bottom()<<'\n';

      CGAL::Bbox_2 bbox = this->convert( rect ).bbox( );

      Point_2 p1(bbox.xmin(), bbox.ymin());
      Point_2 p2(bbox.xmax(), bbox.ymax());

      static X_monotone_curve_2 top = makeSegment(bbox.xmin(), bbox.ymin(), bbox.xmax(), bbox.ymin(), Traits());
      static X_monotone_curve_2 bottom = makeSegment(bbox.xmin(), bbox.ymax(), bbox.xmax(), bbox.ymax(), Traits());
      static X_monotone_curve_2 left = makeSegment(bbox.xmin(), bbox.ymin(), bbox.xmin(), bbox.ymax(), Traits());
      static X_monotone_curve_2 right = makeSegment(bbox.xmax(), bbox.ymin(), bbox.xmax(), bbox.ymax(), Traits());

      CGAL::insert(*(this->arr), top.curve());
      CGAL::insert(*(this->arr), bottom.curve());
      CGAL::insert(*(this->arr), left.curve());
      CGAL::insert(*(this->arr), right.curve());



      static bool firstEntry = true;
      Polynomial_2 polynomial_1;
      Polynomial_2 polynomial_2;
      Polynomial_2 x = CGAL::shift(Polynomial_2(1),1,0);
      Polynomial_2 y = CGAL::shift(Polynomial_2(1),1,1);

      if ( firstEntry )
      {
        firstEntry = false;
        polynomial_1 = CGAL::ipower(y, 2)-200*x - 2000000;
        polynomial_2 = CGAL::ipower(y, 2)+200*x - 2000000;

        Curve_2 cv1 = construct_curve(polynomial_1);
        Curve_2 cv2 = construct_curve(polynomial_2);

        CGAL::insert( *(this->arr), cv1 );
        CGAL::insert( *(this->arr), cv2 );
      }
#endif
      // QColor color = this->backgroundColor;
      // if ( f->color().isValid() )
      // {
      //   color = f->color();
      // }
      // QBrush oldBrush = painter->brush( );
      // QPen oldPen = painter->pen( );
      // painter->setBrush( color );
      // oldPen.setCosmetic(true);
      // painter->setPen(oldPen);
      // painter->drawRect(rect);
      // painter->setBrush( oldBrush );
      std::cout<<"Leaving paintFace Arr_algebraic_segment_traits_2 unbounded"<<std::endl;
      return;
    }

    // Only bounded face is drawn

    std::cout<<"In paintFace Arr_algebraic_segment_traits_2 bounded"<<std::endl;
    QVector< QPointF > pts; // holds the points of the polygon

    typedef typename CGAL::Arr_algebraic_segment_traits_2< Coefficient_ >
                                                        Traits;
    typedef typename Traits::CKvA_2                       CKvA_2;
    typedef std::pair< double, double > Coord_2;
    boost::optional < Coord_2 > p1, p2;
    typedef std::vector< Coord_2 > Coord_vec_2;
    std::list<Coord_vec_2> points;

    std::cout<<"In setupFacade\n";
    typedef Curve_renderer_facade<CKvA_2> Facade;
    QGraphicsView* view = this->scene->views( ).first( );
    int height = view->height();
    int width = view->width();

    QRectF viewport = this->viewportRect( );
    CGAL::Bbox_2 bbox = this->convert( viewport ).bbox( );
    Facade::setup(bbox, view->width(), view->height());    
    std::cout<<"Leaving setupFacade\n";
    /* running with around the outer of the face and generate from it
     * polygon
     */
    Ccb_halfedge_circulator cc=f->outer_ccb();
    do {
      double src_x = CGAL::to_double(cc->source()->point().x());
      double src_y = CGAL::to_double(cc->source()->point().y());
      double tgt_x = CGAL::to_double(cc->target()->point().x());
      double tgt_y = CGAL::to_double(cc->target()->point().y());

      std::cout<< "In while loop: \n";
      std::cout<<"source: "<< src_x <<"\t"<<src_y<<std::endl;
      std::cout<<"target: "<< tgt_x <<"\t"<<tgt_y<<std::endl;

      X_monotone_curve_2 curve = cc->curve();
      Facade::instance().draw( curve, points, &p1, &p2 );

      if(points.empty())
      {
        std::cout<<"In paintFace: points.empty() == True\n";
        continue;
      }

      QVector< QPointF > face_curve_points;
      typename std::list<Coord_vec_2>::const_iterator lit = points.begin();
      while(lit != points.end()) 
      {
        const Coord_vec_2& vec = *lit;
        QPointF first = view->mapToScene( QPoint(vec[0].first, height-vec[0].second) );
        std::cout<<" First point in the vec: ";
        std::cout<<first.x() << "\t" << first.y()<< std::endl;

        QPointF last = view->mapToScene( QPoint(vec[vec.size()-1].first, height-vec[vec.size()-1].second) );
        std::cout<<" Last point in the vec: ";
        std::cout<<last.x() << "\t" << last.y()<< std::endl;

        typename Coord_vec_2::const_iterator vit = vec.begin();
        
        double sceneRectWidth = this->scene->width();
        double sceneRectHeight = this->scene->height();

        bool isPushBack = true;
        while ( vit != vec.end() )
        {
          QPoint coord( vit->first + sceneRectWidth/2, height - vit->second -sceneRectHeight/2);
          QPointF qpt = view->mapToScene( coord );
          if ( src_x < tgt_x )
          {
            face_curve_points.push_back( qpt );
            isPushBack = true;
          }
          else if ( src_x > tgt_x )
          {
            face_curve_points.push_front( qpt );
            isPushBack = false;
          }
          else // src_x == tgt_x
          {
            if (isPushBack)
            {
              face_curve_points.push_back( qpt );
            }
            else
            {
              face_curve_points.push_front( qpt );
            }
          }

          vit++;
          // std::cout << qpt.x() << "\t" << qpt.y() << std::endl;
        }
        lit++;
      }
      pts += face_curve_points;
      points.clear();
      face_curve_points.clear();
      //created from the outer boundary of the face
    } while (++cc != f->outer_ccb());

    // make polygon from the outer ccb of the face 'f'
    QPolygonF pgn (pts);

    // FIXME: get the bg color
    QColor color = this->backgroundColor;
    if ( f->color().isValid() )
    {
      color = f->color();
    }

    QBrush oldBrush = painter->brush( );
    QPen oldPen = painter->pen();
    painter->setBrush( color );
    painter->setPen( this->edgesPen );

    painter->drawPolygon( pgn );

    painter->setBrush( oldBrush );
    painter->setPen( oldPen );

    std::cout<<"Leaving paintFace Arr_algebraic_segment_traits_2 bounded"<<std::endl;
 }

  template < typename Kernel_ >
  void paintFace(Face_handle f, QPainter* painter,
                 CGAL::Arr_linear_traits_2< Kernel_ > /* traits */)
  {
    std::cout<<"In paintFace Arr_linear_traits_2"<<std::endl;

    if (!f->is_unbounded())  // f is not the unbounded face
    {
      std::cout<<"In paintFace Arr_linear_traits_2 bounded"<<std::endl;
      QVector< QPointF > pts; // holds the points of the polygon

      /* running with around the outer of the face and generate from it
       * polygon
       */
      Ccb_halfedge_circulator cc=f->outer_ccb();
      do {
        double x = CGAL::to_double(cc->source()->point().x());
        double y = CGAL::to_double(cc->source()->point().y());
        QPointF coord_source(x , y);
        pts.push_back(coord_source );
        //created from the outer boundary of the face
      } while (++cc != f->outer_ccb());

      // make polygon from the outer ccb of the face 'f'
      QPolygonF pgn (pts);

      // FIXME: get the bg color
      QColor color = this->backgroundColor;
      if ( f->color().isValid() )
      {
        color = f->color();
      }
      QPen oldPen = painter->pen( );
      oldPen.setCosmetic(true);
      QBrush oldBrush = painter->brush( );
      painter->setBrush( color );
      painter->setPen(oldPen);
      painter->drawPolygon( pgn );
      painter->setBrush( oldBrush );
    }
    else
    {
      std::cout<<"In paintFace Arr_linear_traits_2 unbounded"<<std::endl;
      // QRectF rect = this->viewportRect( );
      // QColor color = this->backgroundColor;
      // painter->fillRect( rect, color );
#if 0
      QRectF rect = this->viewportRect( );
      std::cout<<rect.left()<<'\t';
      std::cout<<rect.right()<<'\t';
      std::cout<<rect.top()<<'\t';
      std::cout<<rect.bottom()<<'\n';

      QColor color = this->backgroundColor;
      if ( f->color().isValid() )
      {
        color = f->color();
      }
      QBrush oldBrush = painter->brush( );
      painter->setBrush( color );
      painter->drawRect(rect);
      painter->setBrush( oldBrush );
#endif
      std::cout<<"Leaving paintFace Arr_linear_traits_2 unbounded"<<std::endl;
    }
  }

#if 0
  void drawDiagnosticArc( QPointF c, QPointF s, QPointF t, QPainter* qp )
  {
    QBrush saveBrush = qp->brush( );
    double r = QLineF( c, s ).length();
    QRectF bb( c.x() - r, c.y() - r, 2*r, 2*r );

    QPainterPath path;
    // because we flipped the y-axis in our view, we need to flip our angles
    // with respect to the y-axis. we can do this by flipping s and t, then
    // negating their y-values
    std::swap( s, t );
    double as = atan2( -(s - c).y(), (s - c).x() );
    double at = atan2( -(t - c).y(), (t - c).x() );
    double aspan = at - as;

    path.moveTo( c );
    path.lineTo( s );
    path.arcTo( bb, as * 180/M_PI, aspan * 180/M_PI );
    path.closeSubpath( );
    qp->drawEllipse( bb );

    qp->setBrush( ::Qt::red );
    qp->fillPath( path, qp->brush() );
    qp->setBrush( saveBrush );

  }
#endif 
  /**
     Return false if the tip of the given curve doesn't align with either of the
     endpoints of the next curve.
  */
  // template < typename CircularKernel >
  bool isProperOrientation( Ccb_halfedge_circulator cc );

  // template < typename CircularKernel >
  bool pathTouchingSource( const QPainterPath& path, X_monotone_curve_2 c );


#if 0
  template < typename CircularKernel >
  void paintFace(Face_handle f, QPainter* painter,
                 CGAL::Arr_circular_arc_traits_2<CircularKernel> /* traits */)
  {
    // std::cout << "face begin" << std::endl;
    if (! f->is_unbounded())  // f is not the unbounded face
    {
      QBrush oldBrush = painter->brush( );
      QColor def_bg_color = this->backgroundColor;
      if (! f->color().isValid())
      {
        painter->setBrush( def_bg_color );
      }
      else
      {
        painter->setBrush( f->color( ) );
      }

      // approach: draw a closed path with arcs
      QPainterPath path;
      //path.setFillRule( ::Qt::WindingFill );

      bool isFirstArc = true;
      Ccb_halfedge_circulator cc=f->outer_ccb();
      do
      {
        if (this->antenna(cc))
          continue;

        Halfedge_handle he = cc;
        X_monotone_curve_2 c = he->curve(); // circular arc

        const typename CircularKernel::Circle_2& circ = c.supporting_circle();
        const typename CircularKernel::Point_2& center = circ.center();
        QPointF source( to_double(c.source().x()), to_double(c.source().y()) );
        QPointF target( to_double(c.target().x()), to_double(c.target().y()) );

        if ( isFirstArc )
        {
          isFirstArc = false;
          // make sure we are going the right way
          Ccb_halfedge_circulator ccnext = cc;
          ccnext++;
          while ( this->antenna( ccnext ) ) ccnext++;
          Halfedge_handle next_he = ccnext;
          X_monotone_curve_2 next_c = next_he->curve( );
          QPointF next_source( to_double(next_c.source().x()),
                               to_double(next_c.source().y()) );
          QPointF next_target( to_double(next_c.target().x()),
                               to_double(next_c.target().y()) );
          // std::cout << "next curve's points: " << std::endl
          //           << "  s: " << next_source.x( )
          //           << " " << next_source.y( ) << std::endl
          //           << "  t: " << next_target.x( )
          //           << " " << next_target.y( ) << std::endl;
          double dist1 = QLineF( target, next_source ).length();
          double dist2 = QLineF( target, next_target ).length();
          if ( dist1 > 1e-2 && dist2 > 1e-2 )
          {
            std::cout << "swapping first curve points" << std::endl;
            std::swap( source, target );
          }

          path.moveTo( source );
        }
        else
        {
          //bool source_is_left = (source.x() < target.x());
          double dist = QLineF( path.currentPosition(), source ).length( );
          // if ( dist > 1e-2 )
          // {
          //     std::cout << "swapping source and target " << dist
          //               << std::endl;
          //     std::swap( source, target );
          // }
        }
        // std::cout << "currentPosition: " << path.currentPosition().x()
        //           << ", " << path.currentPosition().y() << std::endl;
        std::stringstream ss;
        ss << "  source: " << to_double(source.x( )) << ", "
           << to_double(source.y()) << std::endl
           << "  target: " << to_double(target.x( )) << ", "
           << to_double(target.y()) << std::endl;

#if 0
        source = QPointF( to_double(c.source().x()), to_double(c.source().y()));
        target = QPointF( to_double(c.target().x()), to_double(c.target().y()));
        double asource = std::atan2( -to_double(source.y() - center.y()),
                                     to_double(source.x() - center.x()));
        double atarget = std::atan2( -to_double(target.y() - center.y()),
                                     to_double(target.x() - center.x()));
#endif
        double asource = std::atan2( to_double(source.y() - center.y()),
                                     to_double(source.x() - center.x()) );
        double atarget = std::atan2( to_double(target.y() - center.y()),
                                     to_double(target.x() - center.x()) );

        ss << "  asource: " << (asource * 180/M_PI) << std::endl
           << "  atarget: " << (atarget * 180/M_PI) << std::endl;

        //std::swap(asource, atarget);
        double aspan = atarget - asource;
        ss << "  aspan: " << (aspan * 180/M_PI) << std::endl;

        if(aspan < 0.)
          aspan += 2 * CGAL_PI;

        const double coeff = 180*16/CGAL_PI;
        double circR = sqrt(to_double(circ.squared_radius()));

#if 0
        painter->drawEllipse( to_double( center.x() - circR ),
                              to_double( center.y() - circR ),
                              2*circR, 2*circR );
#endif
        QPointF curPos = path.currentPosition( );
        //path.lineTo( to_double(target.x( )), to_double(target.y()) );

#if 0
        ss << "drawing line from " << curPos.x( ) << ", " << curPos.y( )
           << " to "
           << to_double(target.x( )) << ", " << to_double(target.y());
#endif
        // std::cout << ss.str( ) << std::endl;
        path.arcTo( convert(circ.bbox()), asource * 180/CGAL_PI,
                    aspan *180/CGAL_PI );
#if 0
        qp->drawArc(convert(circ.bbox()),
                    (int)(asource * coeff),
                    (int)(aspan * coeff));
#endif
      } while (++cc != f->outer_ccb());

      // fill the face according to its color (stored at any of her
      // incidents curves)
#if 0
      painter->drawPolygon( pgn );
#endif
      path.closeSubpath( );
#if 0
      QPainterPathStroker stroker;
      QPainterPath fillablePath = stroker.createStroke( path );
#endif
      painter->fillPath( path, painter->brush() );
#if 0
      QPolygonF fillIt = path.toFillPolygon( );
      painter->drawPolygon( fillIt );
#endif
      //painter->drawPath( path );
      //painter->fillPath( path, painter->brush() );
      painter->setBrush( oldBrush );
    }
    else
    {
      QRectF rect = this->viewportRect( );
      QColor color = this->backgroundColor;
      painter->fillRect( rect, color );
    }
  }
#endif

  //void cacheCurveBoundingRects( );

}; // class ArrangementGraphicsItem

template < typename Arr_, class ArrTraits >
ArrangementGraphicsItem< Arr_, ArrTraits >::
ArrangementGraphicsItem( Arrangement* arr_ ):
  arr( arr_ ),
  painterostream( 0 )
{
  if ( this->arr->number_of_vertices( ) == 0 )
  {
    this->hide( );
  }

  this->updateBoundingBox( );
  this->setZValue( 3 );
}

template < typename Arr_, typename ArrTraits >
QRectF
ArrangementGraphicsItem< Arr_, ArrTraits >::
boundingRect( ) const
{
  QRectF rect = this->convert( this->bb );
  return rect;
}

template < typename Arr_, typename ArrTraits >
void
ArrangementGraphicsItem< Arr_, ArrTraits >::
paint(QPainter* painter,
      const QStyleOptionGraphicsItem* /* option */,
      QWidget*  /*widget*/)
{
  this->paint( painter, ArrTraits( ) );
}

template < typename Arr_, typename ArrTraits >
template < typename TTraits >
void ArrangementGraphicsItem< Arr_, ArrTraits >::
paint(QPainter* painter, TTraits /* traits */)
{
  std::cout<<"In paint ArrTraits"<<std::endl;

  this->paintFaces( painter );

  painter->setPen( this->verticesPen );

  this->painterostream =
    ArrangementPainterOstream< Traits >( painter, this->boundingRect( ) );
  this->painterostream.setScene( this->scene );

  QRectF rect = this->boundingRect( );
  std::cout<<"Curve boundingRect rect\n";
  std::cout<<"left, right, bottom, top:\n";
  std::cout<<rect.left()<<", "<<rect.right()<<", "<<rect.bottom()<<", "<<rect.top()<<std::endl;

  for ( Vertex_iterator it = this->arr->vertices_begin( );
        it != this->arr->vertices_end( ); ++it )
  {
    Point_2 p = it->point( );
    Kernel_point_2 pt( p.x( ), p.y( ) );
    this->painterostream << pt;
  }

  painter->setPen( this->edgesPen );
  for ( Edge_iterator it = this->arr->edges_begin( );
        it != this->arr->edges_end( ); ++it )
  {
    X_monotone_curve_2 curve = it->curve( );

    Bbox_2 bbox = curve.bbox();
    std::cout<<"Curve bounding box\n";
    std::cout<<"xmin, xmax, ymin, ymax:\n";
    std::cout<<bbox.xmin()<<", "<<bbox.xmax()<<", "<<bbox.ymin()<<", "<<bbox.ymax()<<std::endl;
    this->painterostream << curve;
  }

  if ( this->firstEntry )
  {
    this->firstEntry = false;
    QEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Left, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), 
      keyEvent);
    std::cout<<"After sending left event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Right, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending right event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Up, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending up event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Down, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending down event\n";
  }
}

template < typename Arr_, typename ArrTraits >
template < typename CircularKernel >
void ArrangementGraphicsItem< Arr_, ArrTraits >::
paint(QPainter* painter,
      CGAL::Arr_circular_arc_traits_2< CircularKernel > /* traits */)
{
  std::cout<<"In paint Arr_circular_arc_traits_2"<<std::endl;
  this->paintFaces( painter );

  typedef Kernel_point_2 Non_arc_point_2;
  typedef typename Traits::Point_2 Arc_point_2;

  painter->setPen( this->verticesPen );
  this->painterostream =
    ArrangementPainterOstream< Traits >( painter, this->boundingRect( ) );
  this->painterostream.setScene( this->scene );

  for ( Vertex_iterator it = this->arr->vertices_begin( );
        it != this->arr->vertices_end( ); ++it )
  {
    Arc_point_2 pt = it->point( );
    Non_arc_point_2 pt2(CGAL::to_double(pt.x( )), CGAL::to_double(pt.y()) );
    this->painterostream << pt2;
  }
  
  painter->setPen( this->edgesPen );
  for ( Edge_iterator it = this->arr->edges_begin( );
        it != this->arr->edges_end( ); ++it )
  {
    X_monotone_curve_2 curve = it->curve( );
    this->painterostream << curve;
  }

  if ( this->firstEntry )
  {
    this->firstEntry = false;
    QEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Left, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), 
      keyEvent);
    std::cout<<"After sending left event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Right, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending right event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Up, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending up event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Down, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending down event\n";
  }
}


template < typename Arr_, typename ArrTraits >
template < typename Coefficient_ >
void ArrangementGraphicsItem< Arr_, ArrTraits >::
paint(QPainter* painter,
      CGAL::Arr_algebraic_segment_traits_2< Coefficient_ > /* traits */)
{
  std::cout<<"In paint Arr_algebraic_segment_traits_2\n";

  // Set up the scale


  QRectF clipRect = this->boundingRect( );
  std::cout<<"left, right, bottom, top:\n";
  std::cout<<clipRect.left()<<", "<<clipRect.right()<<", "<<clipRect.bottom()<<", "<<clipRect.top()<<std::endl;

  QList< QGraphicsView* > views = this->scene->views( );
  if ( views.size( ) == 0 )
  {
    std::cout<<"views.size( ) == 0\n";
  }

  QGraphicsView* view = views.first( );

  // view->resetMatrix();
  if ( !std::isinf(clipRect.left( )) &&
       !std::isinf(clipRect.right( )) &&
       !std::isinf(clipRect.top( )) &&
       !std::isinf(clipRect.bottom( )) )
  {
    std::cout<<"In If with finite bound\n";
    // view->resetMatrix();
    // double viewHeight = view->height();
#if 0
    double viewHeight = 456;
    std::cout<<"viewHeight: "<<viewHeight<<std::endl;

    double boundingRectHeight = clipRect.bottom() - clipRect.top();

    view->scale(1/this->scalingFactor, 1/this->scalingFactor);
    this->scalingFactor = (0.5 * viewHeight) / boundingRectHeight;
    view->scale(scalingFactor, scalingFactor);
#endif
    std::cout<<"Leaving If with finite bound\n";
  }
  else
  {
    std::cout<<"In If with infinite bound\n";
    clipRect = this->viewportRect( );
    std::cout<<"Leaving If with infinite bound\n"; 
  }

  std::cout<<"left, right, bottom, top:\n";
  std::cout<<clipRect.left()<<", "<<clipRect.right()<<", "<<clipRect.bottom()<<", "<<clipRect.top()<<std::endl;

  // paint the faces for the purpose of brushing
  this->paintFaces( painter );

  // paint the curve itself
  painter->setPen( this->verticesPen );
  this->painterostream =
    ArrangementPainterOstream< Traits >( painter, clipRect );
  this->painterostream.setScene( this->scene );

  std::cout<<"After initializing painterostream\n";

  for ( Vertex_iterator it = this->arr->vertices_begin( );
        it != this->arr->vertices_end( ); ++it )
  {
    Point_2 p = it->point( );
    //std::pair< double, double > approx = p.to_double( );
    //Kernel_point_2 pt( approx.first, approx.second );
    //this->painterostream << pt;
    this->painterostream << p;
  }

  std::cout<<"After done with vertices\n";

  painter->setPen( this->edgesPen );
  for ( Edge_iterator it = this->arr->edges_begin( );
        it != this->arr->edges_end( ); ++it )
  {
    X_monotone_curve_2 curve = it->curve( );
    this->painterostream << curve;
  }

  if ( this->firstEntry )
  {
    this->firstEntry = false;
#if 0
    QEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Left, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), 
      keyEvent);
    std::cout<<"After sending left event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Right, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending right event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Up, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending up event\n";

    keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Down, ::Qt::NoModifier);
    QCoreApplication::postEvent(getCurrentView(), keyEvent);
    std::cout<<"After sending down event\n";
#endif
    QEvent *keyEvent = NULL;

    for (int i=0; i<80; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Left, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<80; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Right, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<80; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Right, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<76; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Left, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<40; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Up, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<40; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Down, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<40; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Down, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }

    for (int i=0; i<39; i++)
    {
      keyEvent = new QKeyEvent(QEvent::KeyPress, ::Qt::Key_Up, ::Qt::NoModifier);
      QCoreApplication::postEvent(getCurrentView(), keyEvent);
    }
  } 
}

// We let the bounding box only grow, so that when vertices get removed
// the maximal bbox gets refreshed in the GraphicsView
template < typename Arr_, typename ArrTraits >
void ArrangementGraphicsItem< Arr_, ArrTraits >::updateBoundingBox( )
{
  this->updateBoundingBox( ArrTraits( ) );
}

template < typename Arr_, typename ArrTraits >
template < typename TTraits >
void ArrangementGraphicsItem< Arr_, ArrTraits >::
updateBoundingBox(TTraits /* traits */)
{
  this->prepareGeometryChange( );
  if ( this->arr->number_of_vertices( ) == 0 )
  {
    this->bb = Bbox_2( 0, 0, 0, 0 );
    this->bb_initialized = false;
    return;
  }
  else
  {
    this->bb = this->arr->vertices_begin( )->point( ).bbox( );
    this->bb_initialized = true;
  }

  for ( Curve_iterator it = this->arr->curves_begin( );
        it != this->arr->curves_end( );
        ++it )
  {
    if ( this->curveBboxMap.count( it ) == 0 )
    {
      this->curveBboxMap[ it ] = it->bbox( );
    }
    this->bb = this->bb + this->curveBboxMap[ it ];
  }
}

template < typename Arr_, typename ArrTraits >
template < typename Kernel_ >
void
ArrangementGraphicsItem< Arr_, ArrTraits >::
updateBoundingBox(CGAL::Arr_linear_traits_2< Kernel_ > /* traits */)
{
  std::cout<<"In updateBoundingBox Arr_linear_traits_2\n";
  this->prepareGeometryChange( );
  QRectF clipRect = this->viewportRect( );
  this->convert = Converter<Kernel>( clipRect );

  std::cout<<"left, right, bottom, top:\n";
  std::cout<<clipRect.left()<<", "<<clipRect.right()<<", "<<clipRect.bottom()<<", "<<clipRect.top()<<std::endl;

  if ( ! clipRect.isValid( ) /*|| this->arr->number_of_vertices( ) == 0*/ )
  {
    this->bb = Bbox_2( 0, 0, 0, 0 );
    this->bb_initialized = false;
    return;
  }
  else
  {
    this->bb = this->convert( clipRect ).bbox( );
    this->bb_initialized = true;
  }

  for ( Curve_iterator it = this->arr->curves_begin( );
        it != this->arr->curves_end( );
        ++it )
  {
    if ( it->is_segment( ) )
    {
      this->bb = this->bb + it->segment( ).bbox( );
    }
    else if ( it->is_ray( ) )
    {
      QLineF qclippedRay = this->convert( it->ray( ) );
      Segment_2 clippedRay = this->convert( qclippedRay );
      this->bb = this->bb + clippedRay.bbox( );
    }
    else // ( it->is_line( ) )
    {
      QLineF qclippedLine = this->convert( it->line( ) );
      Segment_2 clippedLine = this->convert( qclippedLine );
      this->bb = this->bb + clippedLine.bbox( );
    }
  }
}

template < typename Arr_, typename ArrTraits >
template < typename Coefficient_ >
void ArrangementGraphicsItem< Arr_, ArrTraits >::
updateBoundingBox(CGAL::Arr_algebraic_segment_traits_2<Coefficient_> traits)
{
  std::cout<<"In updateBoundingBox Arr_algebraic_segment_traits_2\n";
  this->prepareGeometryChange( );
  if ( this->arr->number_of_vertices( ) == 0 )
  {
    double inf = std::numeric_limits<double>::infinity();
    this->bb = Bbox_2( -inf, -inf, inf, inf );
    this->bb_initialized = false;
    std::cout<<"Leaving updateBoundingBox no vertex\n";
    return;
  }
  else
  {
    //std::pair< double, double > approx =
    //  this->arr->vertices_begin( )->point( ).to_double( );
    //this->bb = CGAL::Bbox_2( approx.first, approx.second,
    //                         approx.first, approx.second );
    this->bb = CGAL::Bbox_2( 0, 0, 0, 0 );
    this->bb_initialized = true;
  }
#if 0
  typename Traits::Make_x_monotone_2 make_x_monotone_2 =
    traits.make_x_monotone_2_object( );
  for ( Curve_iterator it = this->arr->curves_begin( );
        it != this->arr->curves_end( );
        ++it )
  {
    std::vector< CGAL::Object > cvs;
    make_x_monotone_2( *it, std::back_inserter( cvs ) );
    for ( unsigned int i = 0 ; i < cvs.size( ); ++i )
    {
      X_monotone_curve_2 cv;
      CGAL::assign( cv, cvs[ i ] );
      this->bb = this->bb + cv.bbox( );
    }
  }
#endif

  int curve_cnt = 0;

  for ( Edge_iterator it = this->arr->edges_begin( );
        it != this->arr->edges_end( ); ++it )
  {
    X_monotone_curve_2 curve = it->curve( );
    this->bb = this->bb + curve.bbox( );
    std::cout<<"In updateBoundingBox for"<<std::endl;
    std::cout<<curve.bbox( ).xmin()<<"\t";
    std::cout<<curve.bbox( ).xmax()<<"\t";
    std::cout<<curve.bbox( ).ymin()<<"\t";
    std::cout<<curve.bbox( ).ymax()<<"\t"<<std::endl;

    curve_cnt++;

  }

  std::cout<<"curve_cnt\t"<<curve_cnt<<std::endl;
  std::cout<<"Leaving updateBoundingBox at the end\n";
}

template < typename Arr_, typename ArrTraits >
void ArrangementGraphicsItem< Arr_, ArrTraits >::modelChanged( )
{
  this->modelChanged( ArrTraits() );
}

template < typename Arr_, typename ArrTraits >
template < typename TTraits >
void ArrangementGraphicsItem< Arr_, ArrTraits >::modelChanged(TTraits )
{
  std::cout<<"In ArrangementGraphicsItem modelChanged"<<std::endl;
  if ( this->arr->is_empty( ) )
  {
    this->hide( );
  }
  else
  {
    this->show( );
  }

  std::cout<<"In ArrangementGraphicsItem modelChanged after if"<<std::endl;
  this->updateBoundingBox( );

  QRectF clipRect = this->boundingRect( );
  std::cout<<"left, right, bottom, top:\n";
  std::cout<<clipRect.left()<<", "<<clipRect.right()<<", "<<clipRect.bottom()<<", "<<clipRect.top()<<std::endl;

  QList< QGraphicsView* > views = this->scene->views( );
  if ( views.size( ) == 0 )
  {
    std::cout<<"views.size( ) == 0\n";
  }

  QGraphicsView* view = views.first( );

  // view->resetMatrix();
  if ( !std::isinf(clipRect.left( )) &&
       !std::isinf(clipRect.right( )) &&
       !std::isinf(clipRect.top( )) &&
       !std::isinf(clipRect.bottom( )) )
  {
    std::cout<<"In If with finite bound\n";

    Ui::ArrangementDemoWindow *ui = getCurrentDemoWindowUi();

    if (!ui->actionDelete->isChecked())
    {
      std::cout<<"actionDelete not checked"<<std::endl;

      view->scale(1.1, 1.1);
      view->scale(1/1.1, 1/1.1);
    }

    std::cout<<"Leaving If with finite bound\n";
  }

  this->update( );
  std::cout<<"Leaving ArrangementGraphicsItem modelChanged"<<std::endl;
}

template < typename Arr_, typename ArrTraits >
template < typename Coefficient_ >
void ArrangementGraphicsItem< Arr_, ArrTraits >::
modelChanged(CGAL::Arr_algebraic_segment_traits_2<Coefficient_> )
{
  std::cout<<"In ArrangementGraphicsItem modelChanged Arr_algebraic_segment_traits_2"<<std::endl;
  if ( this->arr->is_empty( ) )
  {
    this->hide( );
  }
  else
  {
    this->show( );
  }

  std::cout<<"In ArrangementGraphicsItem modelChanged Arr_algebraic_segment_traits_2 after if"<<std::endl;
  this->updateBoundingBox( );

  QRectF clipRect = this->boundingRect( );
  std::cout<<"left, right, bottom, top:\n";
  std::cout<<clipRect.left()<<", "<<clipRect.right()<<", "<<clipRect.bottom()<<", "<<clipRect.top()<<std::endl;

  QList< QGraphicsView* > views = this->scene->views( );
  if ( views.size( ) == 0 )
  {
    std::cout<<"views.size( ) == 0\n";
  }

  QGraphicsView* view = views.first( );

  // view->resetMatrix();
  if ( !std::isinf(clipRect.left( )) &&
       !std::isinf(clipRect.right( )) &&
       !std::isinf(clipRect.top( )) &&
       !std::isinf(clipRect.bottom( )) )
  {
    std::cout<<"In If with finite bound\n";
    // view->resetMatrix();
    // double viewHeight = view->height();

    // QList<QWidget*> qWidgetList = QApplication::topLevelWidgets();
    // extern class ArrangementDemoWindow;

    Ui::ArrangementDemoWindow *ui = getCurrentDemoWindowUi();
          // dynamic_cast<Ui::ArrangementDemoWindow>qWidgetList.first();
    // ArrangementDemoWindow *myWindw;
    // std::cout<<Ui::ArrangementDemoWindow::SEGMENT_TRAITS;
    double viewHeight = 456;
    std::cout<<"viewHeight: "<<viewHeight<<std::endl;

    if (!ui->actionDelete->isChecked())
    {
      std::cout<<"actionDelete not checked"<<std::endl;
      double boundingRectHeight = clipRect.bottom() - clipRect.top();

      view->scale(1/this->scalingFactor, 1/this->scalingFactor);
      this->scalingFactor = (0.5 * viewHeight) / boundingRectHeight;
      view->scale(this->scalingFactor, this->scalingFactor);
    }

    std::cout<<"Leaving If with finite bound\n";
  }

  this->update( );
  std::cout<<"Leaving ArrangementGraphicsItem modelChanged Arr_algebraic_segment_traits_2"<<std::endl;
}

template < typename Arr_, typename ArrTraits >
void
ArrangementGraphicsItem< Arr_, ArrTraits >::
paintFace( Face_handle f, QPainter* painter )
{
  std::cout<<"In paintFace entry\n";

  if ( f->visited( ) )
  {
    return;
  }

  std::cout<<"In paintFace f->visited( ) == false"<<std::endl;

  int holes = 0;
  int inner_faces = 0;
  Holes_iterator hit; // holes iterator
  this->paintFace( f, painter, Traits( ) );
  f->set_visited( true );

  std::cout<< "After this->paintFace( f, painter, Traits( ) )\n";

  for ( hit = f->holes_begin(); hit != f->holes_end(); ++hit )
  {
    std::cout<<"Inside holes_begin loop\n";
    // Traverse in clockwise order
    Ccb_halfedge_circulator cc = *hit;
    do {
      Halfedge_handle he = cc;
      Halfedge_handle he2 = he->twin();
      Face_handle inner_face = he2->face();
      if ( this->antenna( he ) )
      {
        continue;
      }

      // move on to next hole
      if ( ! inner_face->visited( ) )
      {
        inner_faces++;
      }

      this->visit_ccb_faces( inner_face, painter );
    } while ( ++cc != *hit );

    holes++;
  }// for
  // if ( f->is_unbounded( ) )
  // {
  //   std::cout << "unbounded face has " << holes << " holes" << std::endl;
  //   std::cout << "unbounded face has " << inner_faces << " inner faces"
  //             << std::endl;
  // }
  // if ( f->is_fictitious( ) )
  // {
  //   std::cout << "fictitious face has " << holes << " holes"
  //             << std::endl;
  //   std::cout << "fictitious face has " << inner_faces << " inner faces"
  //             << std::endl;
  // }
  std::cout<<"Leaving paintFace( Face_handle f, QPainter* painter )\n";

}

template < typename Arr_, typename ArrTraits >
bool
ArrangementGraphicsItem< Arr_, ArrTraits >::
isProperOrientation( Ccb_halfedge_circulator cc )
{
  Ccb_halfedge_circulator ccnext = cc;
  Halfedge_handle he = cc;
  X_monotone_curve_2 thisCurve = he->curve( );
  ccnext++;
  while ( this->antenna( ccnext ) ) ccnext++;
  Halfedge_handle next_he = ccnext;
  X_monotone_curve_2 nextCurve = next_he->curve( );

  QPointF thisTarget( to_double(thisCurve.target().x()),
                      to_double(thisCurve.target().y()) );
  QPointF nextSource( to_double(nextCurve.source().x()),
                      to_double(nextCurve.source().y()) );
  QPointF nextTarget( to_double(nextCurve.target().x()),
                      to_double(nextCurve.target().y()) );
  double dist1 = QLineF( thisTarget, nextSource ).length();
  double dist2 = QLineF( thisTarget, nextTarget ).length();
  bool res = ( dist1 < 1e-2 || dist2 < 1e-2 );

  return res;
}

template < typename Arr_, typename ArrTraits >
bool 
ArrangementGraphicsItem< Arr_, ArrTraits >::
pathTouchingSource( const QPainterPath& path, X_monotone_curve_2 c )
{
  QPointF a = path.currentPosition( );
  QPointF b( to_double(c.source().x()), to_double(c.source().y()) );
  // QPointF d( to_double(c.target().x()), to_double(c.target().y()) );
  bool res = (QLineF( a, b ).length() < 1e-2);

  return res;
}

template < typename Arr_, typename ArrTraits >
template < typename Kernel_ >
void 
ArrangementGraphicsItem< Arr_, ArrTraits >::
paintFace( Face_handle f, QPainter* painter,
                CGAL::Arr_segment_traits_2< Kernel_ > )
{
  std::cout<<"In paintFace Arr_segment_traits_2"<<std::endl;

  if (!f->is_unbounded())  // f is not the unbounded face
  {
    std::cout<<"In paintFace Arr_segment_traits_2 bounded"<<std::endl;
    QVector< QPointF > pts; // holds the points of the polygon

    /* running with around the outer of the face and generate from it
     * polygon
     */
    Ccb_halfedge_circulator cc=f->outer_ccb();
    do {
      double x = CGAL::to_double(cc->source()->point().x());
      double y = CGAL::to_double(cc->source()->point().y());
      QPointF coord_source(x , y);
      pts.push_back(coord_source );
      //created from the outer boundary of the face
    } while (++cc != f->outer_ccb());

    // make polygon from the outer ccb of the face 'f'
    QPolygonF pgn (pts);

    // FIXME: get the bg color
    QColor color = this->backgroundColor;
    if (color == ::Qt::white )
    {
      std::cout<<"In paintFace Arr_segment_traits_2 bounded, bg color is white\n";
    }
    if ( f->color().isValid() )
    {
      std::cout<<"In paintFace Arr_segment_traits_2 bounded, f->color() is Valid\n";
      color = f->color();

      if ( color == ::Qt::white )
      {
        std::cout<<"In paintFace Arr_segment_traits_2 bounded, f->color() is white\n";
      }

      if ( color == ::Qt::black )
      {
        std::cout<<"In paintFace Arr_segment_traits_2 bounded, f->color() is black\n";
      }
    }

    QBrush oldBrush = painter->brush( );
    QPen pen = painter->pen();
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush( color );
    painter->drawPolygon( pgn );
    painter->setBrush( oldBrush );
  }
  else
  {
    std::cout<<"In paintFace Arr_segment_traits_2 unbounded"<<std::endl;
    QRectF rect = this->viewportRect( );
    std::cout<<rect.left()<<'\t';
    std::cout<<rect.right()<<'\t';
    std::cout<<rect.top()<<'\t';
    std::cout<<rect.bottom()<<'\n';

    QColor color = this->backgroundColor;
    if ( f->color().isValid() )
    {
      color = f->color();
    }
    QBrush oldBrush = painter->brush( );
    QPen pen = painter->pen();
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush( color );
    painter->drawRect(rect);
    painter->setBrush( oldBrush );

    std::cout<<"Leaving paintFace Arr_segment_traits_2 unbounded"<<std::endl;
  }
}

template < typename Arr_, typename ArrTraits >
template < typename Kernel_ >
void 
ArrangementGraphicsItem< Arr_, ArrTraits >::
paintFace( Face_handle f, QPainter* painter,
                CGAL::Arr_polyline_traits_2< Kernel_ > )
{
  std::cout<<"In paintFace Arr_polyline_traits_2"<<std::endl;

  if (!f->is_unbounded())  // f is not the unbounded face
  {
    std::cout<<"In paintFace Arr_polyline_traits_2 bounded"<<std::endl;
    typedef typename CGAL::Arr_polyline_traits_2<Kernel_> Arr_poly_traits;
    typedef typename Arr_poly_traits::Compare_endpoints_xy_2 Comp_end_pts_2;
    typedef typename Arr_poly_traits::Construct_min_vertex_2 Poly_const_min_v;
    typedef typename Arr_poly_traits::Construct_max_vertex_2 Poly_const_max_v;

    // Obtain a polyline traits class and construct the needed functors
    Arr_poly_traits poly_tr;
    Comp_end_pts_2 comp_end_pts = poly_tr.compare_endpoints_xy_2_object();
    Poly_const_min_v poly_const_min_v=poly_tr.construct_min_vertex_2_object();
    Poly_const_max_v poly_const_max_v=poly_tr.construct_max_vertex_2_object();

    // Construct needed functors from the segment traits
    typedef typename Arr_poly_traits::Subcurve_traits_2      Subcurve_traits;
    typedef typename Subcurve_traits::Construct_min_vertex_2 Seg_const_min_v;
    typedef typename Subcurve_traits::Construct_max_vertex_2 Seg_const_max_v;
    Seg_const_min_v construct_min_v = poly_tr.subcurve_traits_2()->
      construct_min_vertex_2_object();
    Seg_const_max_v construct_max_v = poly_tr.subcurve_traits_2()->
      construct_max_vertex_2_object();

    // Iterator of the segments of an x-monotone polyline
    typename X_monotone_curve_2::Subcurve_const_iterator seg_it;

    QVector< QPointF > pts; // holds the points of the polygon
    X_monotone_curve_2 cv;

    /* running with around the outer of the face and generate from it
     * polygon
     */
    Ccb_halfedge_circulator cc = f->outer_ccb();
    do {
      // The drawing is actually identical to segment
      double x = CGAL::to_double(cc->source()->point().x());
      double y = CGAL::to_double(cc->source()->point().y());
      QPointF coord_source(x , y);
      pts.push_back(coord_source );

      // The code below is problematic
      // cv = cc->curve();

      // // Determine the direction of cv (left-to-right or right-to-left)
      // Comparison_result dir = comp_end_pts(cv);

      // for (seg_it = cv.subcurves_begin();
      //      seg_it != cv.subcurves_end() ; ++seg_it)
      //   {
      //     if (dir == SMALLER)
      //       {
      //         // cv is directed from left-to-right
      //         // Adding the left-min vertex of the current segment
      //         double x = CGAL::to_double((construct_min_v(*seg_it)).x());
      //         double y = CGAL::to_double((construct_min_v(*seg_it)).y());
      //         QPointF coord_source(x , y);
      //         pts.push_back(coord_source );
      //       }
      //     else
      //       {
      //         // cv is directed from right-to-left
      //         // Adding the right-max vertex of the current segment
      //         double x = CGAL::to_double((construct_max_v(*seg_it)).x());
      //         double y = CGAL::to_double((construct_max_v(*seg_it)).y());
      //         QPointF coord_source(x , y);
      //         pts.push_back(coord_source );
      //       }
      //   }

      // if (dir == SMALLER)
      //   {
      //     // Add the right-most point of cv
      //     double x = CGAL::to_double((poly_const_max_v(cv)).x());
      //     double y = CGAL::to_double((poly_const_max_v(cv)).y());
      //     QPointF coord_source(x , y);
      //     pts.push_back(coord_source );
      //   }
      // else
      //   {
      //     // Add the left-most point of cv
      //     double x = CGAL::to_double((poly_const_min_v(cv)).x());
      //     double y = CGAL::to_double((poly_const_min_v(cv)).y());
      //     QPointF coord_source(x , y);
      //     pts.push_back(coord_source );
      //   }
      //created from the outer boundary of the face
    } while (++cc != f->outer_ccb());

    // make polygon from the outer ccb of the face 'f'
    QPolygonF pgn( pts );

    // fill the face according to its color (stored at any of her
    // incidents curves)
    QBrush oldBrush = painter->brush( );
    QColor def_bg_color = this->backgroundColor;
    if (! f->color().isValid())
    {
      painter->setBrush( def_bg_color );
    }
    else
    {
      painter->setBrush( f->color( ) );
    }
    QPen pen = painter->pen();
    pen.setCosmetic(true);
    painter->setPen(pen);

    painter->drawPolygon( pgn );
    painter->setBrush( oldBrush );
  }
  else
  {
    std::cout<<"In paintFace Arr_polyline_traits_2 unbounded"<<std::endl;
    QRectF rect = this->viewportRect( );
    std::cout<<rect.left()<<'\t';
    std::cout<<rect.right()<<'\t';
    std::cout<<rect.top()<<'\t';
    std::cout<<rect.bottom()<<'\n';

    QColor color = this->backgroundColor;
    if ( f->color().isValid() )
    {
      color = f->color();
    }
    QBrush oldBrush = painter->brush( );
    QPen pen = painter->pen();
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush( color );
    painter->drawRect(rect);
    painter->setBrush( oldBrush );

    std::cout<<"Leaving paintFace Arr_polyline_traits_2 unbounded"<<std::endl;
  }
}

template < typename Arr_, typename ArrTraits >
template < typename CircularKernel >
void 
ArrangementGraphicsItem< Arr_, ArrTraits >::
paintFace(Face_handle f, QPainter* painter,
               CGAL::Arr_circular_arc_traits_2<CircularKernel> /* traits */)
{
  std::cout<<"In paintFace Arr_circular_arc_traits_2"<<std::endl;

  if ( f->is_unbounded( ) )
  {

    std::cout<<"In paintFace Arr_circular_arc_traits_2 unbounded"<<std::endl;
    QRectF rect = this->viewportRect( );
    std::cout<<rect.left()<<'\t';
    std::cout<<rect.right()<<'\t';
    std::cout<<rect.top()<<'\t';
    std::cout<<rect.bottom()<<'\n';

    QColor color = this->backgroundColor;
    if ( f->color().isValid() )
    {
      color = f->color();
    }
    QBrush oldBrush = painter->brush( );
    QPen pen = painter->pen();
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush( color );
    painter->drawRect(rect);
    painter->setBrush( oldBrush );

    std::cout<<"Leaving paintFace Arr_circular_arc_traits_2 unbounded"<<std::endl;
    return;
  }

  std::cout<<"In paintFace Arr_circular_arc_traits_2 bounded"<<std::endl;
  QVector< QPointF > pts;
  QPainterPath path;
  bool isFirstArc = true;
  Ccb_halfedge_circulator cc=f->outer_ccb();
  int curve_cnt = 0;

  typedef CGAL::Arr_circular_arc_traits_2<CircularKernel> Traits;
  typedef CircularKernel                                Kernel;
  typedef typename Kernel::Root_of_2                    Root_of_2;

  Arr_compute_y_at_x_2< Traits > compute_y_at_x_2;

  do
  {
    if (this->antenna(cc))
    {
      continue;
    }
    curve_cnt++;

    Halfedge_handle he = cc;
    X_monotone_curve_2 c = he->curve();
    // Get the co-ordinates of the curve's source and target.
    double sx = CGAL::to_double(he->source()->point().x()),
      sy = CGAL::to_double(he->source()->point().y()),
      tx = CGAL::to_double(he->target()->point().x()),
      ty = CGAL::to_double(he->target()->point().y());

    QPointF coord_source(sx, sy);
    QPointF coord_target(tx, ty);

    // Transform the point coordinates from general coordinate system to
    // Qt scene coordinate system
    QPoint coord_source_viewport = this->fromScene( coord_source );
    QPoint coord_target_viewport = this->fromScene( coord_target );

    if (false)
    {
      pts.push_back(coord_source );
    }
    else
    {
      // If the curve is monotone, than its source and its target has the
      // extreme x co-ordinates on this curve.
      bool is_source_left = (sx < tx);
      int  x_min = is_source_left ?
        coord_source_viewport.x( ) : coord_target_viewport.x( );
      int  x_max = is_source_left ?
        coord_target_viewport.x( ) : coord_source_viewport.x( );
      double curr_x, curr_y;
      int x;

      pts.push_back(coord_source );

      // Draw the curve as pieces of small segments
      const int DRAW_FACTOR = 5;
      if (is_source_left)
      {
        for ( x = x_min + DRAW_FACTOR; x < x_max; x+=DRAW_FACTOR )
        {
          //= COORD_SCALE)
          curr_x = this->toScene( x );

          // If curr_x > x_max or curr_x < x_min
          if (curr_x < CGAL::to_double(c.left().x()) || curr_x > CGAL::to_double(c.right().x()))
          {
            continue;
          }

          curr_y = compute_y_at_x_2.approx(c, Root_of_2(curr_x));
          pts.push_back( QPointF( curr_x, curr_y ) );
        }// for
      }
      else
      {
        for ( x = x_max; x > x_min; x-=DRAW_FACTOR )
        {
          curr_x = this->toScene( x );
          if (curr_x < CGAL::to_double(c.left().x()) 
            || curr_x > CGAL::to_double(c.right().x()))
          {
            continue;
          }

          curr_y = compute_y_at_x_2.approx(c, Root_of_2(curr_x));
          pts.push_back( QPointF( curr_x, curr_y ) );
        }// for
      }// else
      pts.push_back(coord_target );
    }
    //created from the outer boundary of the face
  } while (++cc != f->outer_ccb());

  std::cout<<"Curve_cnt: "<<curve_cnt<<std::endl;
  std::cout<<"pts size: "<<pts.size()<<std::endl;
 
  QPolygonF pgn( pts );

  // fill the face according to its color (stored at any of her
  // incidents curves)
  QBrush oldBrush = painter->brush( );
  QColor def_bg_color = this->backgroundColor;
  if (! f->color().isValid())
  {
    painter->setBrush( def_bg_color );
  }
  else
  {
    painter->setBrush( f->color( ) );
  }

  QPen pen = painter->pen();
  pen.setCosmetic(true);
  painter->setPen(pen);
  painter->drawPolygon( pgn );
  painter->setBrush( oldBrush ); 

  std::cout<<"Leaving paintFace Arr_circular_arc_traits_2 bounded"<<std::endl;
}
#if 0
  template < typename Arr_, typename ArrTraits >
  template < typename Kernel_ >
  void ArrangementGraphicsItem< Arr_, ArrTraits >::
  paint(QPainter* painter, CGAL::Arr_linear_traits_2< Kernel_ > /* traits */)
  {
    std::cout<<"In paint Arr_linear_traits_2"<<std::endl;
    this->updateBoundingBox( );
    this->paintFaces( painter );
    painter->setPen( this->verticesPen );

    this->painterostream =
      ArrangementPainterOstream< Traits >( painter, this->boundingRect( ) );
    this->painterostream.setScene( this->scene );

    for ( Vertex_iterator it = this->arr->vertices_begin( );
          it != this->arr->vertices_end( ); ++it )
    {
      Point_2 pt = it->point( );
      this->painterostream << pt;
    }

    painter->setPen( this->edgesPen );
    for ( Edge_iterator it = this->arr->edges_begin( );
          it != this->arr->edges_end( ); ++it )
    {
      X_monotone_curve_2 curve = it->curve( );
      this->painterostream << curve;
    }
  }
#endif
  
 

#if 0
  template < typename Arr_, typename ArrTraits >
  template < typename Coefficient_ >
  void ArrangementGraphicsItem< Arr_, ArrTraits >::
  paint(QPainter* painter,
        CGAL::Arr_algebraic_segment_traits_2< Coefficient_ > /* traits */)
  {
    // this->paintFaces( painter );

    std::cout<<"In paint Arr_algebraic_segment_traits_2 after paintFaces"<<std::endl;
    painter->setPen( this->verticesPen );
    QRectF clipRect = this->viewportRect( );
    // if ( std::isinf(clipRect.left( )) ||
    //      std::isinf(clipRect.right( )) ||
    //      std::isinf(clipRect.top( )) ||
    //      std::isinf(clipRect.bottom( )) )
    // {
    //   std::cout<<"In if isinf == true"<<std::endl;
    //   clipRect = this->viewportRect( );
    // }

    std::cout<<"In paint after if"<<std::endl;
    std::cout<<clipRect.left()<<"\t";
    std::cout<<clipRect.right()<<"\t";
    std::cout<<clipRect.bottom()<<"\t";
    std::cout<<clipRect.top()<<"\t"<<std::endl;

    std::cout<<"In paint Arr_algebraic_segment_traits_2 before painterostream"<<std::endl;
    this->painterostream =
      ArrangementPainterOstream< Traits >( painter, clipRect );
    this->painterostream.setScene( this->scene );

    int point_cnt = 0;
    for ( Vertex_iterator it = this->arr->vertices_begin( );
          it != this->arr->vertices_end( ); ++it )
    {
      Point_2 p = it->point( );

      std::pair< double, double > p_x_y = p.to_double();

      std::cout<<p_x_y.first<<"\t"<<p_x_y.second<<std::endl;
      point_cnt++;
      // std::pair< double, double > approx = p.to_double( );
      Kernel_point_2 pt( p_x_y.first, p_x_y.second );
      this->painterostream << pt;
      // this->painterostream << p;
    }

    std::cout<<"point_cnt:\t"<<point_cnt<<std::endl;

    int curve_cnt = 0;
    painter->setPen( this->edgesPen );
    for ( Edge_iterator it = this->arr->edges_begin( );
          it != this->arr->edges_end( ); ++it )
    {
      X_monotone_curve_2 curve = it->curve( );
      curve_cnt++;
      this->painterostream << curve;
    }

    std::cout<<"curve_cnt:\t"<<curve_cnt<<std::endl;
  }
#endif



#if 0
  template < typename Arr_, typename ArrTraits >
  template < typename Coefficient_ >
  void ArrangementGraphicsItem< Arr_, ArrTraits >::
  updateBoundingBox(CGAL::Arr_algebraic_segment_traits_2<Coefficient_> traits)
  {
    this->prepareGeometryChange( );
    if ( this->arr->number_of_vertices( ) == 0 )
    {
      this->bb = Bbox_2( 0, 0, 0, 0 );
      this->bb_initialized = false;
      // return;
    }
    else
    {
      //std::pair< double, double > approx =
      //  this->arr->vertices_begin( )->point( ).to_double( );
      //this->bb = CGAL::Bbox_2( approx.first, approx.second,
      //                         approx.first, approx.second );
      this->bb = CGAL::Bbox_2( 0, 0, 0, 0 );
      this->bb_initialized = true;
    }

    std::cout<<"In updateBoundingBox Arr_algebraic_segment_traits_2 after if"<<std::endl;
#if 0
    typename Traits::Make_x_monotone_2 make_x_monotone_2 =
      traits.make_x_monotone_2_object( );

    std::cout<<"In updateBoundingBox Arr_algebraic_segment_traits_2 after make_x_monotone_2"<<std::endl;

    int curve_cnt = 0;


    for ( Curve_iterator it = this->arr->curves_begin( );
          it != this->arr->curves_end( );
          ++it )
    {
      curve_cnt++;
      std::vector< CGAL::Object > cvs;
      make_x_monotone_2( *it, std::back_inserter( cvs ) );

      std::cout<<"cvs.size\t"<<cvs.size()<<std::endl;

      // for ( unsigned int i = 0 ; i < cvs.size( ); ++i )
      // {
      //   X_monotone_curve_2 cv;
      //   CGAL::assign( cv, cvs[ i ] );
      //   this->bb = this->bb + cv.bbox( );
      // }
    }
#endif
    int curve_cnt = 0;

    for ( Edge_iterator it = this->arr->edges_begin( );
          it != this->arr->edges_end( ); ++it )
    {
      X_monotone_curve_2 curve = it->curve( );
      this->bb = this->bb + curve.bbox( );
      std::cout<<"In updateBoundingBox for"<<std::endl;
      std::cout<<curve.bbox( ).xmin()<<"\t";
      std::cout<<curve.bbox( ).xmax()<<"\t";
      std::cout<<curve.bbox( ).ymin()<<"\t";
      std::cout<<curve.bbox( ).ymax()<<"\t"<<std::endl;
      curve_cnt++;
      // this->painterostream << curve;
    }

    std::cout<<"In updateBoundingBox after for"<<std::endl;
    std::cout<<this->bb.xmin()<<"\t";
    std::cout<<this->bb.xmax()<<"\t";
    std::cout<<this->bb.ymin()<<"\t";
    std::cout<<this->bb.ymax()<<"\t"<<std::endl;

    // QRectF qBoundingRect = this->boundingRect();
    // QRectF viewportRect  = this->viewportRect();

    // double x_min = qBoundingRect.left();
    // double x_max = std::isinf(qBoundingRect.right())? viewportRect.right(): qBoundingRect.right();
    // double y_min = qBoundingRect.bottom();
    // double y_max = std::isinf(qBoundingRect.top())? viewportRect.top(): qBoundingRect.top();

    // this->bb = Bbox_2( x_min, y_min, x_max, y_max );

    std::cout<<"curve_cnt\t"<<curve_cnt<<std::endl;
  }
#endif


  /**
     Specialized methods:
     updateBoundingBox
  */
#if 0
  template < typename Arr_, typename Kernel_ >
  class ArrangementGraphicsItem< Arr_, CGAL::Arr_linear_traits_2< Kernel_ > > :
    public ArrangementGraphicsItemBase
  {
    typedef Arr_ Arrangement;
    typedef ArrangementGraphicsItemBase Superclass;
    typedef typename Arrangement::Geometry_traits_2     Traits;
    typedef typename Arrangement::Vertex_iterator       Vertex_iterator;
    typedef typename Arrangement::Curve_iterator        Curve_iterator;
    typedef typename Arrangement::Edge_iterator         Edge_iterator;
    typedef typename ArrTraitsAdaptor< Traits >::Kernel Kernel;
    typedef typename Traits::X_monotone_curve_2         X_monotone_curve_2;
    typedef typename Kernel::Point_2                    Point_2;
    typedef typename Kernel::Segment_2                  Segment_2;

  public:
    ArrangementGraphicsItem( Arrangement* arr_ ):
      arr( arr_ ),
      painterostream( 0 )
    {
      if ( this->arr->number_of_vertices( ) == 0 ) {
        this->hide( );
      }
      this->updateBoundingBox( );
      this->setZValue( 3 );
    }

  public: // methods
#if 0
    void modelChanged( )
    {
      if ( this->arr->is_empty( ) )
      {
        this->hide( );
      }
      else
      {
        this->show( );
      }
      this->updateBoundingBox( );
      this->update( );
    }

    // @override QGraphicsItem::boundingRect
    QRectF boundingRect( ) const
    {
      QRectF rect = this->convert( this->bb );
      return rect;
    }
#endif

    // @override QGraphicsItem::paint
#if 0
    virtual void paint( QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* widget )
    {
      this->updateBoundingBox( );
      painter->setPen( this->verticesPen );
      this->painterostream =
        ArrangementPainterOstream< Traits >( painter, this->boundingRect( ) );
      this->painterostream.setScene( this->scene );

      for ( Vertex_iterator it = this->arr->vertices_begin( );
            it != this->arr->vertices_end( ); ++it )
      {
        Point_2 pt = it->point( );
        this->painterostream << pt;
      }
      painter->setPen( this->edgesPen );
      for ( Edge_iterator it = this->arr->edges_begin( );
            it != this->arr->edges_end( ); ++it )
      {
        X_monotone_curve_2 curve = it->curve( );
        this->painterostream << curve;
      }
    }
#endif

  protected: // methods
#if 0
    void updateBoundingBox( )
    {
      this->prepareGeometryChange( );
      QRectF clipRect = this->viewportRect( );
      this->convert = Converter<Kernel>( clipRect );

      if ( ! clipRect.isValid( ) /*|| this->arr->number_of_vertices( ) == 0*/ )
      {
        this->bb = Bbox_2( 0, 0, 0, 0 );
        this->bb_initialized = false;
        return;
      }
      else
      {
        this->bb = this->convert( clipRect ).bbox( );
        this->bb_initialized = true;
      }

      for ( Curve_iterator it = this->arr->curves_begin( );
            it != this->arr->curves_end( );
            ++it )
      {
        if ( it->is_segment( ) )
        {
          this->bb = this->bb + it->segment( ).bbox( );
        }
        else if ( it->is_ray( ) )
        {
          QLineF qclippedRay = this->convert( it->ray( ) );
          Segment_2 clippedRay = this->convert( qclippedRay );
          this->bb = this->bb + clippedRay.bbox( );
        }
        else // ( it->is_line( ) )
        {
          QLineF qclippedLine = this->convert( it->line( ) );
          Segment_2 clippedLine = this->convert( qclippedLine );
          this->bb = this->bb + clippedLine.bbox( );
        }
      }
    }
#endif

  protected: // fields
    Arrangement* arr;
    ArrangementPainterOstream< Traits > painterostream;
    CGAL::Qt::Converter< Kernel > convert;
  }; // class ArrangementGraphicsItem
#endif

} // namespace Qt
} // namespace CGAL

#endif // CGAL_QT_ARRANGEMENT_GRAPHICS_ITEM_H
