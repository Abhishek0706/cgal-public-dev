//! \file Scene.h
#ifndef SCENE_H
#define SCENE_H
#include "config.h"
#include "Scene_config.h"

#include <CGAL/Three/Scene_interface.h>
#include <CGAL/Three/Scene_draw_interface.h>

#include <QtOpenGL/qgl.h>
#include <QAbstractListModel>
#include <QString>
#include <QColor>
#include <QList>
#include <QItemDelegate>
#include <QPixmap>
#include <QItemSelection>
#include <QGLViewer/qglviewer.h>

#include <iostream>
#include <cmath>
#include <boost/variant.hpp>

class QEvent;
class QMouseEvent;
namespace GlSplat { class SplatRenderer; }
namespace CGAL { namespace Three{ class Viewer_interface; }}

class SCENE_EXPORT Scene  :
  public QAbstractListModel, public CGAL::Three::Scene_interface, public CGAL::Three::Scene_draw_interface
{
  Q_OBJECT
  Q_PROPERTY(int numberOfEntries READ numberOfEntries)

  friend class SceneDelegate;

public:
  enum Columns { NameColumn = 0, 
                 ColorColumn, 
                 RenderingModeColumn, 
                 VisibleColumn,
                 ABColumn,
                 LastColumn = ABColumn,
                 NumberOfColumns = LastColumn + 1};
  Scene(QObject*  parent);
  ~Scene();

  //!Adds item to the items list, gives it an ID and
  //!updates the bounding box if needed.
  int addItem(Scene_item* item);
  //!Sets item as the item at index and calls @ref Scene_item#changed().

  //!If emit_item_about_to_be_destroyed is set to true, emits
  //!an itemAboutToBeDestroyed signal.
  Scene_item* replaceItem(int index, Scene_item* item, bool emit_item_about_to_be_destroyed = false);
  /*! Deletes the item with the target index.
   * @returns  the index of the polyhedra just before the
   * one that is erased, or just after. -1 if
   * the list is empty.
   */
  Q_INVOKABLE int erase(int);  

  /*! Deletes the items with the target indexes.
   * @returns the index of the polyhedra just before the
   * one that is erased, or just after. Returns -1 if
   * the list is empty.
   */
  int erase(QList<int>);

  /*! Duplicate a scene item.
   * @returns the ID of the new item (-1 on error).
   */
  int duplicate(int index); 

  // Accessors (getters)
  //! @returns the number of items.
  int numberOfEntries() const;
  //! @returns the list of items.
  const QList<Scene_item*>& entries() const { return m_entries; }
  //! @returns the item at the target index.
  Q_INVOKABLE Scene_item* item(int) const ;
  //! @returns the id of the target item.
  Item_id item_id(Scene_item*) const;
  
  //! \todo Replace Index based selection functionality with those
  //! functions.
  ///@{
  Scene_item* selectedItem() const;
  QList<Scene_item*> selectedItems() const;
  QList<Scene_item*> selectionA() const;
  QList<Scene_item*> selectionB() const;
  ///@}

  //!@returns the currently selected item's index.
  int mainSelectionIndex() const;
  //!@returns the list of currently selected items indices.
  QList<int> selectionIndices() const;
  //!@returns the index of the Item_A
  int selectionAindex() const;
  //!@returns the index of the Item_B
  int selectionBindex() const;

  /*! Is called by Viewer::initializeGL(). Allows all the initialization
   * of OpenGL code that needs a context.
   */
  void initializeGL();
  /*! Is called by Viewer::draw(). Is deprecated and does nothing.*/
  void draw();
  /*! Is deprecated and does nothing.*/
  void drawWithNames();
  /*! Is called by Viewer::draw(Viewer_interface*). Calls draw_aux(false, viewer).
   * @see draw_aux(bool with_names, Viewer_interface).*/
  void draw(CGAL::Three::Viewer_interface*);
  /*! Is called by Viewer::drawWithNames(Viewer_interface*). Calls draw_aux(true, viewer).
   * @see draw_aux(bool with_names, Viewer_interface).*/
  void drawWithNames(CGAL::Three::Viewer_interface*);
  /*! Manages the key events.
   * @returns true if the keyEvent executed well.
   */
  bool keyPressEvent(QKeyEvent* e);

  //!@returns the scene bounding box
  Bbox bbox() const;
  float get_bbox_length() const;
  //!@returns the length of the bounding box's diagonal.
  double len_diagonal() const
  {
    Bbox box = bbox();
    double dx = box.xmax - box.xmin;
    double dy = box.ymax - box.ymin;
    double dz = box.zmax - box.zmin;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
  }

  // QAbstractItemModel functions
  //!@returns the number of items, which is also the sumber of rows in the sceneView.
  int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  //!@returns the number of columns in the sceneView.
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  //!@returns the column data corresponding to role.
  QVariant data ( const QModelIndex & index, int role = ::Qt::DisplayRole ) const;
  //!@returns the type of data correspondind to the role.
  QVariant headerData ( int section, ::Qt::Orientation orientation, int role = ::Qt::DisplayRole ) const;
  //!@returns the flags for the item at the target index.
  ::Qt::ItemFlags flags ( const QModelIndex & index ) const;
  /*! Sets the column data for the target index. Returns false if index is not valid and
   * if role is not EditRole.*/
  bool setData(const QModelIndex &index, const QVariant &value, int role);

  // auxiliary public function for QMainWindow
  //!Selects the row at index i in the sceneView.
  QItemSelection createSelection(int i);
  //!Selects all the rows in the sceneView.
  QItemSelection createSelectionAll();

public Q_SLOTS:
  /*! This is an overloaded function.
   * Notifies the scene that the sender item was modified.
   * Called by the items. Calls @ref Scene_item#changed().
   * This function is called by the items.*/
  void itemChanged();
  /*! Notifies the scene that the item at index i was modified.
   * Called by the items. Calls @ref Scene_item#changed().
   * This function is called by the items.*/
  void itemChanged(int i); 
  /*! Notifies the scene that the item was modified.
   *  Calls @ref Scene_item#changed().
   * This function is called by the items.*/
  void itemChanged(Scene_item*);
  //! Sets the selected item to the target index.
  void setSelectedItemIndex(int i)
  {
    selected_item = i;
  }
  //! Sets the selected item to the target index and emits selectionChanged(i).
  void setSelectedItem(int i )
  {
    selected_item = i;
    Q_EMIT selectionChanged(i);
  };
  //! Sets the target item as selected and emits setSelectedItem for its index.
  void setSelectedItem(Scene_item* item_to_select)
  {
    int i=0;
    Q_FOREACH(Scene_item* item, m_entries)
    {
      if (item==item_to_select)
      {
        Q_EMIT setSelectedItem(i);
        break;
      }
      ++i;
    }
  };
  //! Sets the target list of indices as the selected indices.
  void setSelectedItemsList(QList<int> l )
  {
    selected_items_list = l;
  };

  // Accessors (setters)
  //!Sets the item at index i to visible or not visible.
  void setItemVisible(int, bool b);
  //!Sets the item_A as the item at index i .
  void setItemA(int i);
  //!Sets the item_B as the item at index i .
  void setItemB(int i);

Q_SIGNALS:
  //generated automatically by moc
  void newItem(int);
  void updated_bbox();
  void updated();
  void itemAboutToBeDestroyed(Scene_item*);
  void selectionRay(double, double, double, double, double, double);
  void selectionChanged(int i);

private Q_SLOTS:
  //! Casts a selection ray and calls the item function select.
  void setSelectionRay(double, double, double, double, double, double);
  void callDraw(){  QGLViewer* viewer = *QGLViewer::QGLViewerPool().begin(); viewer->update();}

private:
  /*! Calls the drawing functions of each visible item according
   * to its current renderingMode. If with_names is true, uses
   * the OpenGL mode GL_WITH_NAMES, essentially used for the picking.*/
  void draw_aux(bool with_names, CGAL::Three::Viewer_interface*);
  //! List of Scene_items.
  typedef QList<Scene_item*> Entries;
  //!List containing all the scene_items.
  Entries m_entries;
  //! Index of the currently selected item.
  int selected_item;
  //!List of indices of the currently selected items.
  QList<int> selected_items_list;
  //!Index of the item_A.
  int item_A;
  //!Index of the item_B.
  int item_B;
  static GlSplat::SplatRenderer* ms_splatting;
  static int ms_splattingCounter;
public:
  static GlSplat::SplatRenderer* splatting();

}; // end class Scene
/*!
 * \brief The SceneDelegate class
 * Handles the columns of the sceneView
 */
class SCENE_EXPORT SceneDelegate : public QItemDelegate
{
public:
  SceneDelegate(QObject * parent = 0)
    : QItemDelegate(parent),
      checkOnPixmap(":/cgal/icons/check-on.png"),
      checkOffPixmap(":/cgal/icons/check-off.png")
  {
  }
//! Handles the clicks on the sceneView
  bool editorEvent(QEvent *event, QAbstractItemModel *model,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index);
  //! Draws the content of the sceneView
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const;

private:
  QPixmap checkOnPixmap;
  QPixmap checkOffPixmap;
  mutable int size;
}; // end class SceneDelegate

#endif // SCENE_H
