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
// $URL: $
// $Id: $
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef CGAL_QT_CURVE_GRAPHICS_ITEM_H
#define CGAL_QT_CURVE_GRAPHICS_ITEM_H

#include "ArrangementPainterOstream.h"
#include "Utils.h"

#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/GraphicsItem.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <QGraphicsScene>
#include <QRectF>

namespace CGAL {
namespace Qt {

class CurveGraphicsItemBase : public GraphicsItem, public IQGraphicsSceneMixin
{
public:
  virtual ~CurveGraphicsItemBase( ) { }

  // override for IQGraphicsSceneMixin
  virtual QGraphicsScene* getScene( ) const;
};

/**
   Draws selected curves and vertices of an arrangement.
*/
template < class ArrTraits >
class CurveGraphicsItem : public CurveGraphicsItemBase
{
public:
  // known curve types
  typedef ArrTraits Traits;
  typedef typename ArrTraitsAdaptor< Traits >::Kernel Kernel;
  typedef typename Traits::Curve_2 Curve_2;
  typedef typename Traits::X_monotone_curve_2 X_monotone_curve_2;
  typedef typename Traits::Point_2 Point_2;
  typedef typename Kernel::Point_2 Kernel_point_2;

public: // ctors
  CurveGraphicsItem( ):
    painterOstream( 0 ),
    boundingBox( 0, 0, 0, 0 ),
    boundingBoxInitialized( false ),
    m_edgeColor( ::Qt::red ),
    m_edgeWidth( 0 ),
    m_vertexColor( ::Qt::red ),
    m_vertexRadius( 3 )
  {
    this->setZValue( 4 );
  }

public: // methods
  virtual void paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* /* option */,
                     QWidget* /* widget */)
  {
    // draw the curves
    QPen edgesPen( this->m_edgeColor, this->m_edgeWidth );
    painter->setPen( edgesPen );
    QRectF clippingRectangle = this->viewportRect( );
    this->painterOstream =
      ArrangementPainterOstream< Traits >( painter, clippingRectangle );
    this->painterOstream.setScene( this->getScene( ) );
    for ( unsigned int i = 0; i < this->curves.size( ); ++i )
    {
      X_monotone_curve_2 curve = this->curves[ i ];
      this->painterOstream << curve;
    }

    // draw the points
    QPen verticesPen( this->m_vertexColor, this->m_vertexRadius );
    painter->setPen( verticesPen );
    for ( unsigned int i = 0; i < this->points.size( ); ++i )
    {
      Point_2 arrPoint = this->points[ i ];
      Kernel_point_2 point( CGAL::to_double( arrPoint.x( ) ),
                            CGAL::to_double( arrPoint.y( ) ) );
      this->painterOstream << point;
    }
  }

  virtual QRectF boundingRect( ) const
  {
    return this->boundingRect( (Traits*) NULL );
  }

  template < class TTraitsType >
  QRectF boundingRect( TTraitsType* /*unused*/ ) const
  {
    QRectF boundingRectangle = this->convert( this->boundingBox );
    return boundingRectangle;
  }

  template < class Coefficient >
  QRectF boundingRect(
    CGAL::Arr_algebraic_segment_traits_2<Coefficient>* /*unused*/ ) const
  {
    return this->viewportRect( );
  }

  void insert( const X_monotone_curve_2& curve )
  {
    this->curves.push_back( curve );

    //this->updateBoundingBox( );
    modelChanged( );
  }

  void insert( const Point_2& point )
  {
    this->points.push_back( point );

    //this->updateBoundingBox( );
    modelChanged( );
  }

  void clear( )
  {
    this->curves.clear( );
    this->points.clear( );

    this->updateBoundingBox( );
  }

public slots:
  void modelChanged( )
  {
    if ( this->curves.size( ) == 0 )
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

  const QColor& edgeColor( ) const
  {
    return this->m_edgeColor;
  }

  void setEdgeColor( const QColor& color )
  {
    this->m_edgeColor = color;
  }

  int edgeWidth( ) const
  {
    return this->m_edgeWidth;
  }

  void setEdgeWidth( int width )
  {
    this->m_edgeWidth = width;
  }

  const QColor& vertexColor( ) const
  {
    return this->m_vertexColor;
  }

  void setVertexColor( const QColor& color )
  {
    this->m_vertexColor = color;
  }

  int vertexRadius( ) const
  {
    return this->m_vertexRadius;
  }

  void setVertexRadius( int radius )
  {
    this->m_vertexRadius = radius;
  }

protected: // methods
  void updateBoundingBox( )
  {
    // tag dispatch
    updateBoundingBox( (Traits*) 0 );
  }

  // catch-all tag dispatch
  template < class TTraits >
  void updateBoundingBox( TTraits* )
  {
    this->prepareGeometryChange( );

    if ( this->curves.size( ) == 0 )
    {
      this->boundingBox = Bbox_2( 0, 0, 0, 0 );
      this->boundingBoxInitialized = false;
      return;
    }
    else
    {
      this->boundingBox = this->curves[ 0 ].bbox( );
      this->boundingBoxInitialized = true;
    }

    for ( unsigned int i = 1; i < this->curves.size( ); ++i )
    {
      this->boundingBox = this->boundingBox + this->curves[ i ].bbox( );
    }

    for ( unsigned int i = 0; i < this->points.size( ); ++i )
    {
      Point_2 pt = this->points[ i ];
      Kernel_point_2 point(CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
      this->boundingBox = this->boundingBox + point.bbox( );
    }
  }

  template < class RatKernel, class AlgKernel, class NtTraits >
  void updateBoundingBox( Arr_Bezier_curve_traits_2< RatKernel, AlgKernel, NtTraits >* )
  {
    std::cout << "TODO: void updateBoundingBox( Arr_Bezier_curve_traits_2< RatKernel, AlgKernel, NtTraits >* )\n";
    if ( this->curves.size( ) == 0 )
    {
      this->boundingBox = Bbox_2( 0, 0, 0, 0 );
      this->boundingBoxInitialized = false;
      return;
    }
    else
    {
      this->boundingBox = this->curves[ 0 ].supporting_curve( ).bbox( );
      this->boundingBoxInitialized = true;
    }

    for ( unsigned int i = 1; i < this->curves.size( ); ++i )
    {
      this->boundingBox = this->boundingBox + this->curves[ i ].supporting_curve( ).bbox( );
    }
  }

  template < class Coefficient >
  void updateBoundingBox( CGAL::Arr_algebraic_segment_traits_2<Coefficient>* /*unused*/ )
  {
    this->boundingBoxInitialized = true;
    QRectF viewRect = this->viewportRect( );
    this->boundingBox = CGAL::Bbox_2( viewRect.left( ),
      viewRect.bottom( ),
      viewRect.right( ),
      viewRect.top( ) );
  }

protected: // fields
  CGAL::Qt::Converter< Kernel > convert;
  ArrangementPainterOstream< Traits > painterOstream;
  std::vector< X_monotone_curve_2 > curves;
  std::vector< Point_2 > points;
  CGAL::Bbox_2 boundingBox;
  bool boundingBoxInitialized;

  QColor m_edgeColor;
  int m_edgeWidth;
  QColor m_vertexColor;
  int m_vertexRadius;

}; // class CurveGraphicsItem

/**
   Specialization of the base template CurveGraphicsItem:

   updateBoundingBox
*/
template < class Kernel_ >
class CurveGraphicsItem< CGAL::Arr_linear_traits_2< Kernel_ > > :
    public GraphicsItem, public IQGraphicsSceneMixin
{
public: // typedefs
  // known curve types
  typedef CGAL::Arr_linear_traits_2< Kernel_ >          Traits;
  typedef Kernel_ Kernel;
  typedef typename Traits::Curve_2                      Curve_2;
  typedef typename Traits::X_monotone_curve_2           X_monotone_curve_2;
  typedef typename Traits::Point_2                      Point_2;
  typedef typename Kernel::Segment_2                    Segment_2;
  typedef typename Kernel::Line_2                       Line_2;
  typedef typename Kernel::Ray_2                        Ray_2;
  typedef typename Kernel::Point_2                      Kernel_point_2;

public: // ctors
  CurveGraphicsItem( ):
    painterOstream( 0 ),
    boundingBox( 0, 0, 0, 0 ),
    boundingBoxInitialized( false )
  {
    this->setZValue( 4 );
  }

  virtual QGraphicsScene* getScene( ) const
  {
    return this->QGraphicsItem::scene( );
  }

public: // methods
  virtual void paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* /* option */,
                     QWidget* /* widget */)
  {
    QRectF clippingRectangle = this->viewportRect( );
    painter->setPen( QPen( ::Qt::red, 0. ) );
    this->painterOstream =
      ArrangementPainterOstream< Traits >( painter, clippingRectangle );
    for ( unsigned int i = 0; i < this->curves.size( ); ++i )
    {
      X_monotone_curve_2 curve = this->curves[ i ];
      this->painterOstream << curve;
    }
    for ( unsigned int i = 0; i < this->points.size( ); ++i )
    {
      Point_2 arrPoint = this->points[ i ];
      Kernel_point_2 point( CGAL::to_double( arrPoint.x( ) ),
                            CGAL::to_double( arrPoint.y( ) ) );
      this->painterOstream << point;
    }
  }

  void insert( const X_monotone_curve_2& curve )
  {
    this->curves.push_back( curve );
    this->updateBoundingBox( );
  }

  void insert( const Point_2& point )
  {
    this->points.push_back( point );
  }

  void clear( )
  {
    this->curves.clear( );
    this->points.clear( );
  }

  QRectF boundingRect( ) const
  {
    QRectF res;
    if ( this->getScene( ) == NULL )
    {
      return res;
    }
    return this->convert( this->boundingBox );
  }

public slots:
  void modelChanged( )
  {
    if ( this->curves.size( ) == 0 )
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

  const QColor& edgeColor( ) const
  {
    return this->m_edgeColor;
  }

  void setEdgeColor( const QColor& color )
  {
    this->m_edgeColor = color;
  }

  int edgeWidth( ) const
  {
    return this->m_edgeWidth;
  }

  void setEdgeWidth( int width )
  {
    this->m_edgeWidth = width;
  }

  const QColor& vertexColor( ) const
  {
    return this->m_vertexColor;
  }

  void setVertexColor( const QColor& color )
  {
    this->m_vertexColor = color;
  }

  int vertexRadius( ) const
  {
    return this->m_vertexRadius;
  }

  void setVertexRadius( int radius )
  {
    this->m_vertexRadius = radius;
  }

protected: // methods
  void updateBoundingBox( )
  {
    this->boundingBoxInitialized = 0;
    this->boundingBox = CGAL::Bbox_2( 0, 0, 0, 0 );
    QRectF clipRect = this->viewportRect( );
    if ( !clipRect.isValid( ) )
    {
      return;
    }
    this->convert = CGAL::Qt::Converter< Kernel >( clipRect );

    bool first = 1;
    for ( unsigned int i = 0; i < curves.size( ); ++i )
    {
      X_monotone_curve_2 curve = curves[ i ];
      if ( curve.is_segment( ) )
      {
        Segment_2 seg = curve.segment( );
        CGAL::Bbox_2 seg_bbox = seg.bbox( );
        if ( first )
        {
          first = 0;
          this->boundingBoxInitialized = 1;
          this->boundingBox = seg_bbox;
        }
        else
        {
          this->boundingBox = this->boundingBox + seg_bbox;
        }
      }
      else if ( curve.is_ray( ) )
      {
        Ray_2 ray = curve.ray( );
        QLineF qclippedRay = this->convert( ray );
        if ( qclippedRay.isNull( ) )
          continue;
        Segment_2 clippedRay = this->convert( qclippedRay );
        if ( first )
        {
          first = 0;
          this->boundingBoxInitialized = 1;
          this->boundingBox = clippedRay.bbox( );
        }
        else
        {
          this->boundingBox = this->boundingBox + clippedRay.bbox( );
        }
      }
      else // curve.is_line( )
      {
        Line_2 line = curve.line( );
        QLineF qclippedLine = this->convert( line );
        if ( qclippedLine.isNull( ) )
          continue;
        Segment_2 clippedLine = this->convert( qclippedLine );
        if ( first )
        {
          first = 0;
          this->boundingBoxInitialized = 1;
          this->boundingBox = clippedLine.bbox( );
        }
        else
        {
          this->boundingBox = this->boundingBox + clippedLine.bbox( );
        }
      }
    }
  }

protected: // fields
  CGAL::Qt::Converter< Kernel > convert;
  ArrangementPainterOstream< Traits > painterOstream;
  std::vector< X_monotone_curve_2 > curves;
  std::vector< Point_2 > points;
  CGAL::Bbox_2 boundingBox;
  bool boundingBoxInitialized;

  QColor m_edgeColor;
  int m_edgeWidth;
  QColor m_vertexColor;
  int m_vertexRadius;
}; // class CurveGraphicsItem

/**
   Draws selected curves and vertices of an arrangement.
*/
template < class RatKernel, class AlgKernel, class NtTraits >
class CurveGraphicsItem< CGAL::Arr_Bezier_curve_traits_2< RatKernel,
  AlgKernel, NtTraits > >
  : public GraphicsItem, public IQGraphicsSceneMixin
{
public:
  // known curve types
  typedef CGAL::Arr_Bezier_curve_traits_2< RatKernel, AlgKernel, NtTraits >
    Traits;
  typedef typename ArrTraitsAdaptor< Traits >::Kernel Kernel;
  typedef typename Traits::Curve_2 Curve_2;
  typedef typename Traits::X_monotone_curve_2 X_monotone_curve_2;
  typedef typename Traits::Point_2 Point_2;
  typedef typename Kernel::Point_2 Kernel_point_2;

  typedef typename Construct_x_monotone_subcurve_2< Traits >::Subcurve
    Subcurve;

public: // ctors
  CurveGraphicsItem( ):
    painterOstream( 0 ),
    boundingBox( 0, 0, 0, 0 ),
    boundingBoxInitialized( false ),
    m_edgeColor( ::Qt::red ),
    m_edgeWidth( 0 ),
    m_vertexColor( ::Qt::red ),
    m_vertexRadius( 3 )
  {
    this->setZValue( 4 );
  }

  virtual QGraphicsScene* getScene( ) const
  {
    return this->QGraphicsItem::scene( );
  }

public: // methods
  virtual void paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* /* option */,
                     QWidget* /* widget */)
  {
    // draw the curves
    QPen edgesPen( this->m_edgeColor, this->m_edgeWidth );
    painter->setPen( edgesPen );
    QRectF clippingRectangle = this->viewportRect( );
    this->painterOstream =
      ArrangementPainterOstream< Traits >( painter, clippingRectangle );
    this->painterOstream.setScene( this->getScene( ) );
    for ( unsigned int i = 0; i < this->curves.size( ); ++i )
    {
      X_monotone_curve_2 curve = this->curves[ i ];
      this->painterOstream << curve;
    }

    // draw the subcurves
    for ( unsigned int i = 0; i < this->subcurves.size( ); ++i )
    {
      std::cout << "painting subcurve\n";
      this->painterOstream << this->subcurves[ i ];
    }

    // draw the points
    QPen verticesPen( this->m_vertexColor, this->m_vertexRadius );
    painter->setPen( verticesPen );
    for ( unsigned int i = 0; i < this->points.size( ); ++i )
    {
      Point_2 arrPoint = this->points[ i ];
      Kernel_point_2 point( CGAL::to_double( arrPoint.x( ) ),
                            CGAL::to_double( arrPoint.y( ) ) );
      this->painterOstream << point;
    }
  }

  virtual QRectF boundingRect( ) const
  {
    QRectF boundingRectangle = this->convert( this->boundingBox );
    return boundingRectangle;
  }

  void insert( const X_monotone_curve_2& curve )
  {
    this->curves.push_back( curve );

    this->updateBoundingBox( );
  }

  void insert( const Point_2& point )
  {
    this->points.push_back( point );

    this->updateBoundingBox( );
  }

  void insert( const Subcurve& subcurve )
  {
    std::cout << "inserting subcurve\n";
    this->subcurves.push_back( subcurve );

    this->updateBoundingBox( );
  }

  void clear( )
  {
    this->subcurves.clear( );
    this->curves.clear( );
    this->points.clear( );

    this->updateBoundingBox( );
  }

public slots:
  void modelChanged( )
  {
    if ( this->curves.size( ) == 0 && this->subcurves.size( ) == 0 )
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

  const QColor& edgeColor( ) const
  {
    return this->m_edgeColor;
  }

  void setEdgeColor( const QColor& color )
  {
    this->m_edgeColor = color;
  }

  int edgeWidth( ) const
  {
    return this->m_edgeWidth;
  }

  void setEdgeWidth( int width )
  {
    this->m_edgeWidth = width;
  }

  const QColor& vertexColor( ) const
  {
    return this->m_vertexColor;
  }

  void setVertexColor( const QColor& color )
  {
    this->m_vertexColor = color;
  }

  int vertexRadius( ) const
  {
    return this->m_vertexRadius;
  }

  void setVertexRadius( int radius )
  {
    this->m_vertexRadius = radius;
  }

protected: // methods
  void updateBoundingBox( )
  {
    if ( this->curves.size( ) == 0 && this->subcurves.size( ) == 0 )
    {
      this->boundingBox = Bbox_2( 0, 0, 0, 0 );
      this->boundingBoxInitialized = false;
      return;
    }
    else if ( this->curves.size( ) > 0 )
    {
      this->boundingBox = this->curves[ 0 ].supporting_curve( ).bbox( );
      this->boundingBoxInitialized = true;
    }
    else
    {
      this->boundingBox = this->subcurves[ 0 ].m_cv.supporting_curve( ).bbox( );
      this->boundingBoxInitialized = true;
    }

    for ( unsigned int i = 0; i < this->curves.size( ); ++i )
    {
      this->boundingBox = this->boundingBox + this->curves[ i ].supporting_curve( ).bbox( );
    }
    for ( unsigned int i = 0; i < this->subcurves.size( ); ++i )
    {
      this->boundingBox = this->boundingBox + this->subcurves[ i ].m_cv.supporting_curve( ).bbox( );
    }
  }

protected: // fields
  CGAL::Qt::Converter< Kernel > convert;
  ArrangementPainterOstream< Traits > painterOstream;
  std::vector< Subcurve > subcurves;
  std::vector< X_monotone_curve_2 > curves;
  std::vector< Point_2 > points;
  CGAL::Bbox_2 boundingBox;
  bool boundingBoxInitialized;

  QColor m_edgeColor;
  int m_edgeWidth;
  QColor m_vertexColor;
  int m_vertexRadius;

}; // class CurveGraphicsItem< CGAL::Arr_Bezier_curve_traits_2< RatKernel,

} // namespace Qt
} // namespace CGAL

#endif // CGAL_QT_CURVE_GRAPHICS_ITEM_H
