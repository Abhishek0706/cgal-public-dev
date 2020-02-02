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
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#include "Callback.h"

#include <QEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

namespace CGAL {
namespace Qt {
//! Constructor of Callback
/*!
 * \param parent QObject
 */
Callback::Callback( QObject* parent ) : QObject( parent ) { }

//! Function to reset the Callback
void Callback::reset( ) { }

//! Function to determine the event type of Callback
/*!
 * \brief Callback::eventFilter
 * \param object QObject
 * \param event QEvent
 * \return
 */
bool Callback::eventFilter( QObject* object, QEvent* event )
{
  if ( event->type( ) == QEvent::GraphicsSceneMouseMove )
  {
    QGraphicsSceneMouseEvent* mouseEvent =
      static_cast< QGraphicsSceneMouseEvent* >( event );
    this->mouseMoveEvent( mouseEvent );
  }
  else if ( event->type( ) == QEvent::GraphicsSceneMousePress )
  {
    QGraphicsSceneMouseEvent* mouseEvent =
      static_cast< QGraphicsSceneMouseEvent* >( event );
    this->mousePressEvent( mouseEvent );
  }
  else if ( event->type( ) == QEvent::GraphicsSceneMouseRelease )
  {
    QGraphicsSceneMouseEvent* mouseEvent =
      static_cast< QGraphicsSceneMouseEvent* >( event );
    this->mouseReleaseEvent( mouseEvent );
  }
  else if ( event->type( ) == QEvent::KeyPress )
  {
    QKeyEvent* keyEvent = static_cast< QKeyEvent* >( event );
    this->keyPressEvent( keyEvent );
  }
  return QObject::eventFilter( object, event );
}

//! Callback function for mouse press event
void Callback::mousePressEvent(QGraphicsSceneMouseEvent* /* event */) { }

//! Callback function for mouse movement
void Callback::mouseMoveEvent(QGraphicsSceneMouseEvent* /* event */) { }

//! Callback Function for mouse button release
void Callback::mouseReleaseEvent(QGraphicsSceneMouseEvent* /* event */) { }

//! Callback Function for pressing a key
void Callback::keyPressEvent(QKeyEvent* /* event */)  { }

//! Callback function for slot model change
void Callback::slotModelChanged( ) { }

} // namespace Qt
} // namespace CGAL
