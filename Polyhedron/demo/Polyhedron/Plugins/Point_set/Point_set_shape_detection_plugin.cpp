#include "config.h"
#include "Scene_points_with_normal_item.h"
#include "Scene_polygon_soup_item.h"
#include "Scene_polyhedron_item.h"
#include <CGAL/Three/Scene_group_item.h>

#include <CGAL/Three/Polyhedron_demo_plugin_helper.h>
#include <CGAL/Three/Polyhedron_demo_plugin_interface.h>
#include <CGAL/Three/Scene_group_item.h>

#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>

#include <CGAL/linear_least_squares_fitting_3.h>

#include <CGAL/Random.h>
#include <CGAL/Timer.h>

#include <CGAL/Shape_detection_3.h>
#include <CGAL/regularize_planes.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>

#include <CGAL/structure_point_set.h>

#include <QObject>
#include <QAction>
#include <QMainWindow>
#include <QApplication>
#include <QtPlugin>
#include <QMessageBox>

#include <boost/foreach.hpp>
#include <boost/function_output_iterator.hpp>

#include "ui_Point_set_shape_detection_plugin.h"


struct build_from_pair
{
  Point_set& m_pts;

  build_from_pair (Point_set& pts) : m_pts (pts) { }

  void operator() (const std::pair<Point_set::Point, Point_set::Vector>& pair)
  {
    m_pts.insert (pair.first, pair.second);
  }


};

typedef CGAL::Exact_predicates_inexact_constructions_kernel Epic_kernel;
typedef Epic_kernel::Point_3 Point;
//typedef CGAL::Point_with_normal_3<Epic_kernel> Point_with_normal;
//typedef std::vector<Point_with_normal> Point_list;
//typedef CGAL::Identity_property_map<Point_with_normal> PointPMap;
//typedef CGAL::Normal_of_point_with_normal_pmap<Epic_kernel> NormalPMap;
using namespace CGAL::Three;
class Polyhedron_demo_point_set_shape_detection_plugin :
  public QObject,
  public Polyhedron_demo_plugin_helper
{
  Q_OBJECT
  Q_INTERFACES(CGAL::Three::Polyhedron_demo_plugin_interface)
  Q_PLUGIN_METADATA(IID "com.geometryfactory.PolyhedronDemo.PluginInterface/1.0")
  QAction* actionDetect;
  QAction* actionEstimateParameters;
  
  typedef Point_set_3<Kernel>::Point_map PointPMap;
  typedef Point_set_3<Kernel>::Vector_map NormalPMap;

  typedef CGAL::Shape_detection_3::Efficient_RANSAC_traits<Epic_kernel, Point_set, PointPMap, NormalPMap> Traits;
  typedef CGAL::Shape_detection_3::Efficient_RANSAC<Traits> Shape_detection;
  
public:
  void init(QMainWindow* mainWindow, CGAL::Three::Scene_interface* scene_interface, Messages_interface*) {
    scene = scene_interface;
    actionDetect = new QAction(tr("Point Set Shape Detection"), mainWindow);
    actionDetect->setObjectName("actionDetect");
    actionEstimateParameters = new QAction(tr("Point Set Shape Detection (parameter estimation)"), mainWindow);
    actionEstimateParameters->setObjectName("actionEstimateParameters");
    autoConnectActions();
  }

  bool applicable(QAction*) const {
    Scene_points_with_normal_item* item =
      qobject_cast<Scene_points_with_normal_item*>(scene->item(scene->mainSelectionIndex()));
    if (item && item->has_normals())
      return true;
    return false;
  }

  QList<QAction*> actions() const {
    return QList<QAction*>() << actionDetect << actionEstimateParameters;
  }

  public Q_SLOTS:
    void on_actionDetect_triggered();
    void on_actionEstimateParameters_triggered();

private:

  typedef Kernel::Plane_3 Plane_3;
  typedef Kernel::Point_3 Point_3;
  typedef Kernel::Vector_3 Vector_3;

  template <typename PointSet, typename OutputIterator>
  void region_growing(PointSet& point_set,
                      double cluster_epsilon,
                      double epsilon,
                      std::size_t Nmin,
                      double maximal_deviation_of_normals,
                      OutputIterator output)
  {
    typedef typename PointSet::Index Index;
    
    typedef CGAL::Search_traits_3<Kernel> SearchTraits_3;
    typedef CGAL::Search_traits_adapter <Index,
                                         typename PointSet::Point_map, SearchTraits_3> Search_traits;
#define QUERY_SPHERE
#ifdef QUERY_SPHERE
    typedef CGAL::Fuzzy_sphere<Search_traits> Sphere;
#endif
    typedef CGAL::Orthogonal_k_neighbor_search<Search_traits> Neighbor_search;
    typedef typename Neighbor_search::Tree Tree;
    typedef typename Neighbor_search::Distance Distance;

    // build kdtree
    Tree tree(point_set.begin(),
              point_set.end(),
              typename Tree::Splitter(),
              Search_traits (point_set.point_map())
              );
    Distance tr_dist(point_set.point_map());
    
    //Initialization structures
    std::vector <int> label_subregion (point_set.size(), -1);
    int class_index = -1;

    for (typename PointSet::iterator it = point_set.begin(); it != point_set.end(); ++ it)
      {
        if (label_subregion[*it] != -1)
          continue;

        label_subregion[*it] = class_index++;

        int conti = 0; 	//for accelerate least_square fitting 

        Plane_3 optimal_plane(point_set.point(*it),
                              point_set.normal(*it));

        //initialization containers
        std::vector<Index> index_container (1, *it);
        std::vector<Index> index_container_former_ring (1, *it);
        std::list<Index> index_container_current_ring;

        //propagation
        bool propagation = true;
        do
          {
            propagation = false;

            for (std::size_t k = 0; k < index_container_former_ring.size(); k++)
              {
                Index point_index = index_container_former_ring[k];

#ifdef QUERY_SPHERE

                std::vector<Index> neighbors;
                Sphere fs (point_set.point(point_index), cluster_epsilon, 0, tree.traits());
                tree.search (std::back_inserter (neighbors), fs);
                
                for (std::size_t nb = 0; nb < neighbors.size(); ++ nb)
                  {
                    Index neighbor_index = neighbors[nb];
#else
                Neighbor_search search(tree, point_set.point(point_index), 10, 0, true, tr_dist);
                
                for (typename Neighbor_search::iterator nit = search.begin();
                     nit != search.end(); ++ nit)
                  {
                    if (nit->second > cluster_epsilon * cluster_epsilon)
                      break;
                    
                    Index neighbor_index = nit->first;
#endif
                    
                    if (label_subregion[neighbor_index] != -1)
                      continue;

                    const Point_3& neighbor = point_set.point(neighbor_index);
                    Point_3 neighbor_projection = optimal_plane.projection(neighbor);
                    double distance = CGAL::squared_distance(neighbor, neighbor_projection);

                    if (distance > epsilon * epsilon
                        || std::fabs(point_set.normal(neighbor_index)
                                     * optimal_plane.orthogonal_vector()) < maximal_deviation_of_normals)
                      continue;

                    label_subregion[neighbor_index] = class_index;
                    propagation = true;
                    index_container_current_ring.push_back(neighbor_index);
                    conti++;

                    if ((conti<50 && conti % 10 == 0) || (conti>50 && conti % 500 == 0))
                      {
                        std::list<Point_3> listp;
                        for (std::size_t pm = 0; pm < index_container.size(); pm++)
                          listp.push_back(point_set.point(index_container[pm]));

                        Plane_3 reajusted_plane;
                        CGAL::linear_least_squares_fitting_3(listp.begin(),
                                                             listp.end(),
                                                             reajusted_plane,
                                                             CGAL::Dimension_tag<0>());
                        optimal_plane = reajusted_plane;
                      }
                  }
              }

            //update containers
            index_container_former_ring.clear();
            for (typename std::list<Index>::iterator lit = index_container_current_ring.begin();
                 lit != index_container_current_ring.end(); ++lit)
              {
                index_container_former_ring.push_back(*lit);
                index_container.push_back(*lit);
              }
            index_container_current_ring.clear();
          }
        while (propagation);

        if (index_container.size() >= Nmin)
          {
            std::vector<Point_3> out;
            for (std::size_t k = 0; k < index_container.size(); k++)
              out.push_back (point_set.point(index_container[k]));
            *(output ++) = out;
          }
        else
          {
            class_index--;
            label_subregion[*it] = -1;
            for (std::size_t k = 0; k < index_container.size(); k++)
              label_subregion[index_container[k]] = -1;
          }
      }
  }


  Kernel::Point_2 to_2d (const Point_3& centroid,
                         const Vector_3& base1,
                         const Vector_3& base2,
                         const Point_3& query)
  {
    Vector_3 v (centroid, query);
    return Kernel::Point_2 (v * base1, v * base2);
  }

  Point_3 to_3d (const Point_3& centroid,
                 const Vector_3& base1,
                 const Vector_3& base2,
                 const Kernel::Point_2& query)
  {
    return centroid + query.x() * base1 + query.y() * base2;
  }

  void build_alpha_shape (Point_set& points, const Point_3& centroid, const Plane_3& plane,
                          Scene_polyhedron_item* item, double epsilon);
  void build_alpha_shape (Point_set& points, boost::shared_ptr<CGAL::Shape_detection_3::Plane<Traits> > plane,
                          Scene_polyhedron_item* item, double epsilon);

}; // end Polyhedron_demo_point_set_shape_detection_plugin

class Point_set_demo_point_set_shape_detection_dialog : public QDialog, private Ui::PointSetShapeDetectionDialog
{
  Q_OBJECT
public:
  Point_set_demo_point_set_shape_detection_dialog(QWidget * /*parent*/ = 0)
  {
    setupUi(this);
  }

  bool region_growing() const { return m_region_growing->isChecked(); }
  double cluster_epsilon() const { return m_cluster_epsilon_field->value(); }
  double epsilon() const { return m_epsilon_field->value(); }
  unsigned int min_points() const { return m_min_pts_field->value(); }
  double normal_tolerance() const { return m_normal_tolerance_field->value(); }
  double search_probability() const { return m_probability_field->value(); }
  double gridCellSize() const { return 1.0; }
  bool detect_plane() const { return planeCB->isChecked(); } 
  bool detect_sphere() const { return sphereCB->isChecked(); } 
  bool detect_cylinder() const { return cylinderCB->isChecked(); } 
  bool detect_torus() const { return torusCB->isChecked(); } 
  bool detect_cone() const { return coneCB->isChecked(); }
  bool generate_colored_point_set() const { return m_one_colored_point_set->isChecked(); }
  bool generate_subset() const { return m_point_subsets->isChecked(); }
  bool generate_alpha() const { return m_alpha_shapes->isChecked(); }
  bool regularize() const { return m_regularize->isChecked(); }
  bool generate_structured() const { return m_generate_structured->isChecked(); }
};

void Polyhedron_demo_point_set_shape_detection_plugin::on_actionDetect_triggered() {

  CGAL::Random rand(time(0));
  const CGAL::Three::Scene_interface::Item_id index = scene->mainSelectionIndex();

  Scene_points_with_normal_item* item =
    qobject_cast<Scene_points_with_normal_item*>(scene->item(index));

  Scene_points_with_normal_item::Bbox bb = item->bbox();
 
  double diam = CGAL::sqrt((bb.xmax()-bb.xmin())*(bb.xmax()-bb.xmin()) + (bb.ymax()-bb.ymin())*(bb.ymax()-bb.ymin()) + (bb.zmax()-bb.zmin())*(bb.zmax()-bb.zmin()));

  if(item)
    {
      // Gets point set
      Point_set* points = item->point_set();

      if(points == NULL)
        return;

      //Epic_kernel::FT diag = sqrt(((points->bounding_box().max)() - (points->bounding_box().min)()).squared_length());

      // Gets options
      Point_set_demo_point_set_shape_detection_dialog dialog;
      if(!dialog.exec())
        return;
      
      scene->setSelectedItem(-1);
      Scene_group_item *subset_items = new Scene_group_item(QString("%1 (point subsets)").arg(item->name()));
      if (dialog.generate_subset())
        scene->addItem(subset_items);
      subset_items->setExpanded(false);
      Scene_group_item *alpha_items = new Scene_group_item(QString("%1 (alpha shapes)").arg(item->name()));
      if (dialog.generate_alpha())
        scene->addItem(alpha_items);
      alpha_items->setExpanded(false);
      Scene_points_with_normal_item *colored_item
        = new Scene_points_with_normal_item;
      colored_item->setName (QString("%1 (shape detection)").arg(item->name()));
      if (dialog.generate_colored_point_set())
        {
          colored_item->point_set()->template add_property_map<unsigned char>("r", 128);
          colored_item->point_set()->template add_property_map<unsigned char>("g", 128);
          colored_item->point_set()->template add_property_map<unsigned char>("b", 128);
          colored_item->point_set()->check_colors();
          scene->addItem(colored_item);
        }

      if (dialog.region_growing())
        {
    
          QApplication::setOverrideCursor(Qt::WaitCursor);

          std::vector<std::vector<Point_3> > planes;

          CGAL::Timer t;
          t.start();
          region_growing(*points,
                         dialog.cluster_epsilon(),
                         dialog.epsilon(),
                         dialog.min_points(),
                         dialog.normal_tolerance(),
                         std::back_inserter (planes));
          t.stop();
          std::cerr << planes.size() << " shape(s) found in "
                    << t.time() << " second(s)" << std::endl;
      
          for (std::size_t i = 0; i < planes.size(); ++ i)
            {
              Scene_points_with_normal_item *point_item = new Scene_points_with_normal_item;
              point_item->point_set()->add_normal_map();

              unsigned char r, g, b;
              r = static_cast<unsigned char>(64 + rand.get_int(0, 192));
              g = static_cast<unsigned char>(64 + rand.get_int(0, 192));
              b = static_cast<unsigned char>(64 + rand.get_int(0, 192));

              for (std::size_t n = 0; n < planes[i].size(); ++ n)
                point_item->point_set()->insert(planes[i][n]);
              point_item->setRbgColor(r, g, b);

              if (dialog.generate_colored_point_set())
                for (std::size_t n = 0; n < planes[i].size(); ++ n)
                  {
                    Point_set::iterator it = colored_item->point_set()->insert(planes[i][n]);
                    colored_item->point_set()->set_color(*it, r, g, b);
                  }

      
              Point_3 centroid;
              Plane_3 plane;
              CGAL::linear_least_squares_fitting_3(planes[i].begin(),
                                                   planes[i].end(),
                                                   plane, centroid,
                                                   CGAL::Dimension_tag<0>());


              point_item->setName (QString("Plane #%1").arg(item->name()));
              if (dialog.generate_alpha ())
                {
                  Scene_polyhedron_item* poly_item = new Scene_polyhedron_item;

                  build_alpha_shape (*(point_item->point_set()), centroid, plane,
                                     poly_item, dialog.cluster_epsilon());
          
                  poly_item->setColor(point_item->color ());
                  poly_item->setName (QString("Alpha shape #%1").arg(item->name()));
                  poly_item->setRenderingMode (Flat);

                  scene->addItem(poly_item);
                  scene->changeGroup(poly_item, alpha_items);
                }

              for(Point_set::iterator it = point_item->point_set()->begin(); it != point_item->point_set()->end(); ++it)
                point_item->point_set()->normal(*it) = plane.orthogonal_vector();

              if (dialog.generate_subset())
                {
                  scene->addItem (point_item);
                  scene->changeGroup(point_item, subset_items);
                }
              else
                delete point_item;
            }
        }
      else
        {
          QApplication::setOverrideCursor(Qt::WaitCursor);

          typedef Point_set::Point_map PointPMap;
          typedef Point_set::Vector_map NormalPMap;

          typedef CGAL::Shape_detection_3::Efficient_RANSAC_traits<Epic_kernel, Point_set, PointPMap, NormalPMap> Traits;
          typedef CGAL::Shape_detection_3::Efficient_RANSAC<Traits> Shape_detection;

          Shape_detection shape_detection;
          shape_detection.set_input(*points, points->point_map(), points->normal_map());

          std::vector<Scene_group_item *> groups;
          groups.resize(5);
          // Shapes to be searched for are registered by using the template Shape_factory
          if(dialog.detect_plane()){
            groups[0] = new Scene_group_item("Planes");
            shape_detection.add_shape_factory<CGAL::Shape_detection_3::Plane<Traits> >();
          }
          if(dialog.detect_cylinder()){
            groups[1] = new Scene_group_item("Cylinders");
            shape_detection.add_shape_factory<CGAL::Shape_detection_3::Cylinder<Traits> >();
          }
          if(dialog.detect_torus()){
            groups[2] = new Scene_group_item("Torus");
            shape_detection.add_shape_factory< CGAL::Shape_detection_3::Torus<Traits> >();
          }
          if(dialog.detect_cone()){
            groups[3] = new Scene_group_item("Cones");
            shape_detection.add_shape_factory< CGAL::Shape_detection_3::Cone<Traits> >();
          }
          if(dialog.detect_sphere()){
            groups[4] = new Scene_group_item("Spheres");
            shape_detection.add_shape_factory< CGAL::Shape_detection_3::Sphere<Traits> >();
          }

          // Parameterization of the shape detection using the Parameters structure.
          Shape_detection::Parameters op;
          op.probability = dialog.search_probability();       // probability to miss the largest primitive on each iteration.
          op.min_points = dialog.min_points();          // Only extract shapes with a minimum number of points.
          op.epsilon = dialog.epsilon();          // maximum euclidean distance between point and shape.
          op.cluster_epsilon = dialog.cluster_epsilon();    // maximum euclidean distance between points to be clustered.
          op.normal_threshold = dialog.normal_tolerance();   // normal_threshold < dot(surface_normal, point_normal); maximum normal deviation.

          // The actual shape detection.
          CGAL::Timer t;
          t.start();
          shape_detection.detect(op);
          t.stop();
    
          std::cout << shape_detection.shapes().size() << " shapes found in "
                    << t.time() << " second(s)" << std::endl;

          if (dialog.regularize ())
            {
              std::cerr << "Regularization of planes... " << std::endl;
              CGAL::regularize_planes (shape_detection, true, true, true, true,
                                       180 * std::acos (op.normal_threshold) / CGAL_PI, op.epsilon);
    
              std::cerr << "done" << std::endl;
            }

          std::map<Kernel::Point_3, QColor> color_map;
    
          //print_message(QString("%1 shapes found.").arg(shape_detection.number_of_shapes()));
          int index = 0;
          BOOST_FOREACH(boost::shared_ptr<Shape_detection::Shape> shape, shape_detection.shapes())
            {
              CGAL::Shape_detection_3::Cylinder<Traits> *cyl;
              cyl = dynamic_cast<CGAL::Shape_detection_3::Cylinder<Traits> *>(shape.get());
              if (cyl != NULL){
                if(cyl->radius() > diam){
                  continue;
                }
              }
        
              Scene_points_with_normal_item *point_item = new Scene_points_with_normal_item;
              point_item->point_set()->add_normal_map();
      
              unsigned char r, g, b;
              r = static_cast<unsigned char>(64 + rand.get_int(0, 192));
              g = static_cast<unsigned char>(64 + rand.get_int(0, 192));
              b = static_cast<unsigned char>(64 + rand.get_int(0, 192));

              BOOST_FOREACH(std::size_t i, shape->indices_of_assigned_points())
                point_item->point_set()->insert(points->point(*(points->begin()+i)));
              point_item->setRbgColor(r, g, b);

              if (dialog.generate_colored_point_set())
                {
                  BOOST_FOREACH(std::size_t i, shape->indices_of_assigned_points())
                    {
                      Point_set::iterator it = colored_item->point_set()->insert(points->point(*(points->begin()+i)));
                      colored_item->point_set()->set_color(*it, r, g, b);
                    }
                }
              
              // Providing a useful name consisting of the order of detection, name of type and number of inliers
              std::stringstream ss;
              if (dynamic_cast<CGAL::Shape_detection_3::Cylinder<Traits> *>(shape.get())){
                CGAL::Shape_detection_3::Cylinder<Traits> * cyl 
                  = dynamic_cast<CGAL::Shape_detection_3::Cylinder<Traits> *>(shape.get());
                ss << item->name().toStdString() << "_cylinder_" << cyl->radius() << "_";
              }
              else if (dynamic_cast<CGAL::Shape_detection_3::Plane<Traits> *>(shape.get()))
                {
                  ss << item->name().toStdString() << "_plane_";

                  boost::shared_ptr<CGAL::Shape_detection_3::Plane<Traits> > pshape
                    = boost::dynamic_pointer_cast<CGAL::Shape_detection_3::Plane<Traits> > (shape);
          
                  Kernel::Point_3 ref = CGAL::ORIGIN + pshape->plane_normal ();

                  if (color_map.find (ref) == color_map.end ())
                    {
                      ref = CGAL::ORIGIN + (-1.) * pshape->plane_normal ();
                      if (color_map.find (ref) == color_map.end ())
                        color_map[ref] = point_item->color ();
                      else
                        point_item->setColor (color_map[ref]);
                    }
                  else
                    point_item->setColor (color_map[ref]);

                  ss << "(" << ref << ")_";
      
                  if (dialog.generate_alpha ())
                    {
                      // If plane, build alpha shape
                      Scene_polyhedron_item* poly_item = new Scene_polyhedron_item;

                      build_alpha_shape (*(point_item->point_set()), pshape,
                                         poly_item, dialog.cluster_epsilon());
          
                      poly_item->setColor(point_item->color ());
                      poly_item->setName(QString("%1%2_alpha_shape").arg(QString::fromStdString(ss.str()))
                                         .arg (QString::number (shape->indices_of_assigned_points().size())));
                      poly_item->setRenderingMode (Flat);

                      scene->addItem(poly_item);
                      if(scene->item_id(groups[0]) == -1)
                        scene->addItem(groups[0]);
                      scene->changeGroup(poly_item, groups[0]);
                    }
                }
              else if (dynamic_cast<CGAL::Shape_detection_3::Cone<Traits> *>(shape.get()))
                ss << item->name().toStdString() << "_cone_";
              else if (dynamic_cast<CGAL::Shape_detection_3::Torus<Traits> *>(shape.get()))
                ss << item->name().toStdString() << "_torus_";
              else if (dynamic_cast<CGAL::Shape_detection_3::Sphere<Traits> *>(shape.get()))
                ss << item->name().toStdString() << "_sphere_";


              ss << shape->indices_of_assigned_points().size();

              //names[i] = ss.str(		
              point_item->setName(QString::fromStdString(ss.str()));
              point_item->setRenderingMode(item->renderingMode());

              if (dialog.generate_subset()){
                scene->addItem(point_item);
                if (dynamic_cast<CGAL::Shape_detection_3::Cylinder<Traits> *>(shape.get()))
                  {
                    if(scene->item_id(groups[1]) == -1)
                      {
                        scene->addItem(groups[1]);
                        scene->changeGroup (groups[1], subset_items);
                      }
                    scene->changeGroup(point_item, groups[1]);
                  }
                else if (dynamic_cast<CGAL::Shape_detection_3::Plane<Traits> *>(shape.get()))
                  {
                    CGAL::Shape_detection_3::Plane<Traits> * plane = dynamic_cast<CGAL::Shape_detection_3::Plane<Traits> *>(shape.get());

                    //set normals for point_item to the plane's normal
                    for(Point_set::iterator it = point_item->point_set()->begin(); it != point_item->point_set()->end(); ++it)
                      point_item->point_set()->normal(*it) = plane->plane_normal();

                    if(scene->item_id(groups[0]) == -1)
                      {
                        scene->addItem(groups[0]);
                        scene->changeGroup (groups[0], subset_items);
                      }
                    scene->changeGroup(point_item, groups[0]);
                  }
                else if (dynamic_cast<CGAL::Shape_detection_3::Cone<Traits> *>(shape.get()))
                  {
                    if(scene->item_id(groups[3]) == -1)
                      {
                        scene->addItem(groups[3]);
                        scene->changeGroup (groups[3], subset_items);
                      }
                    scene->changeGroup(point_item, groups[3]);
                  }
                else if (dynamic_cast<CGAL::Shape_detection_3::Torus<Traits> *>(shape.get()))
                  {
                    if(scene->item_id(groups[2]) == -1)
                      {
                        scene->addItem(groups[2]);
                        scene->changeGroup (groups[2], subset_items);
                      }
                    scene->changeGroup(point_item, groups[2]);
                  }
                else if (dynamic_cast<CGAL::Shape_detection_3::Sphere<Traits> *>(shape.get()))
                  {
                    if(scene->item_id(groups[4]) == -1)
                      {
                        scene->addItem(groups[4]);
                        scene->changeGroup (groups[4], subset_items);
                      }
                    scene->changeGroup(point_item, groups[4]);
                  }
              }
              else
                delete point_item;

              ++index;
            }
          Q_FOREACH(Scene_group_item* group, groups)
            if(group && group->getChildren().empty())
              delete group;

          if (dialog.generate_structured ())
            {
              std::cerr << "Structuring point set... ";
        
              Scene_points_with_normal_item *pts_full = new Scene_points_with_normal_item;
              pts_full->point_set()->add_normal_map();
              CGAL::structure_point_set (points->begin (), points->end (),
                                         points->point_map(), points->normal_map(),
                                         boost::make_function_output_iterator (build_from_pair ((*(pts_full->point_set())))),
                                         shape_detection,
                                         op.cluster_epsilon);
              if (pts_full->point_set ()->empty ())
                delete pts_full;
              else
                {
                  pts_full->point_set ()->unselect_all();
                  pts_full->setName(tr("%1 (structured)").arg(item->name()));
                  pts_full->setRenderingMode(PointsPlusNormals);
                  pts_full->setColor(Qt::blue);
                  scene->addItem (pts_full);
                }
              std::cerr << "done" << std::endl;
            }
    
        }
    
      if (!(dialog.generate_subset()))
        delete subset_items;
    
      if (!(dialog.generate_alpha()))
        delete alpha_items;

      if (!(dialog.generate_colored_point_set()))
        delete colored_item;
      
      // Updates scene
      scene->itemChanged(index);

      QApplication::restoreOverrideCursor();

      //     Warn user, maybe choice of parameters is unsuitable
      //         if (nb_points_to_remove > 0)
      //         {
      //           QMessageBox::information(NULL,
      //                                    tr("Points selected for removal"),
      //                                    tr("%1 point(s) are selected for removal.\nYou may delete or reset the selection using the item context menu.")
      //                                    .arg(nb_points_to_remove));
      //         }
      item->setVisible(false);
    }
}

void Polyhedron_demo_point_set_shape_detection_plugin::build_alpha_shape
(Point_set& points,  boost::shared_ptr<CGAL::Shape_detection_3::Plane<Traits> > plane,
 Scene_polyhedron_item* item, double epsilon)
{
  typedef Kernel::Point_2  Point_2;
  typedef CGAL::Alpha_shape_vertex_base_2<Kernel> Vb;
  typedef CGAL::Alpha_shape_face_base_2<Kernel>  Fb;
  typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
  typedef CGAL::Delaunay_triangulation_2<Kernel,Tds> Triangulation_2;
  typedef CGAL::Alpha_shape_2<Triangulation_2>  Alpha_shape_2;


  std::vector<Point_2> projections;
  projections.reserve (points.size ());

  for (Point_set::const_iterator it = points.begin(); it != points.end(); ++ it)
    projections.push_back (plane->to_2d (points.point(*it)));

  Alpha_shape_2 ashape (projections.begin (), projections.end (), epsilon);
  
  std::map<Alpha_shape_2::Vertex_handle, std::size_t> map_v2i;

  Scene_polygon_soup_item *soup_item = new Scene_polygon_soup_item;
  
  soup_item->init_polygon_soup(points.size(), ashape.number_of_faces ());
  std::size_t current_index = 0;

  for (Alpha_shape_2::Finite_faces_iterator it = ashape.finite_faces_begin ();
       it != ashape.finite_faces_end (); ++ it)
    {
      if (ashape.classify (it) != Alpha_shape_2::INTERIOR)
        continue;

      for (int i = 0; i < 3; ++ i)
        {
          if (map_v2i.find (it->vertex (i)) == map_v2i.end ())
            {
              map_v2i.insert (std::make_pair (it->vertex (i), current_index ++));
              Point p = plane->to_3d (it->vertex (i)->point ());
              soup_item->new_vertex (p.x (), p.y (), p.z ());
            }
        }
      soup_item->new_triangle (map_v2i[it->vertex (0)],
                               map_v2i[it->vertex (1)],
                               map_v2i[it->vertex (2)]);
    }

  soup_item->orient();
  soup_item->exportAsPolyhedron (item->polyhedron());

  if (soup_item->isEmpty ())
    {
      std::cerr << "POLYGON SOUP EMPTY" << std::endl;
      for (std::size_t i = 0; i < projections.size (); ++ i)
        std::cerr << projections[i] << std::endl;
      
    }
  
  delete soup_item;
}

void Polyhedron_demo_point_set_shape_detection_plugin::on_actionEstimateParameters_triggered() {

  CGAL::Random rand(time(0));
  const CGAL::Three::Scene_interface::Item_id index = scene->mainSelectionIndex();

  Scene_points_with_normal_item* item =
    qobject_cast<Scene_points_with_normal_item*>(scene->item(index));

  if(item)
    {
      // Gets point set
      Point_set* points = item->point_set();

      if(points == NULL)
        return;

      if (points->nb_selected_points() == 0)
        {
          QMessageBox::information(NULL,
                                   tr("Warning"),
                                   tr("Selection is empty.\nTo estimate parameters, please select a planar section."));
          return;
        }
      
      QApplication::setOverrideCursor(Qt::WaitCursor);

      typedef CGAL::Search_traits_3<Kernel> SearchTraits_3;
      typedef CGAL::Search_traits_adapter <Point_set::Index,
                                           Point_set::Point_map, SearchTraits_3> Search_traits;
      typedef CGAL::Orthogonal_k_neighbor_search<Search_traits> Neighbor_search;
      typedef typename Neighbor_search::Tree Tree;
      typedef typename Neighbor_search::Distance Distance;

      // build kdtree
      Tree tree(points->first_selected(),
                points->end(),
                typename Tree::Splitter(),
                Search_traits (points->point_map())
                );
      Distance tr_dist(points->point_map());

      Plane_3 plane;
      CGAL::linear_least_squares_fitting_3(boost::make_transform_iterator
                                           (points->first_selected(),
                                            CGAL::Property_map_to_unary_function<Point_set::Point_map>
                                            (points->point_map())),
                                           boost::make_transform_iterator
                                           (points->end(),
                                            CGAL::Property_map_to_unary_function<Point_set::Point_map>
                                            (points->point_map())),
                                           plane,
                                           CGAL::Dimension_tag<0>());

      std::vector<double> epsilon, dispersion, cluster_epsilon;

      Vector_3 norm = plane.orthogonal_vector();
      norm = norm / std::sqrt (norm * norm);
      for (Point_set::iterator it = points->first_selected(); it != points->end(); ++ it)
        {
          double dist = CGAL::squared_distance (plane, points->point(*it));
          epsilon.push_back(dist);

          double disp = std::fabs (norm * points->normal(*it));
          dispersion.push_back (disp);

          Neighbor_search search(tree, points->point(*it), 2, 0, true, tr_dist);
          typename Neighbor_search::iterator nit = search.begin();
          ++ nit;
          double eps = nit->second;
          cluster_epsilon.push_back(eps);
        }

      std::sort (epsilon.begin(), epsilon.end());
      std::sort (dispersion.begin(), dispersion.end());
      std::sort (cluster_epsilon.begin(), cluster_epsilon.end());
      
      QApplication::restoreOverrideCursor();

      
      QMessageBox::information(NULL,
                               tr("Estimated Parameters"),
                               tr("Epsilon = [%1 ; %2 ; %3 ; %4 ; %5]\nNormal Tolerance = [%6 ; %7 ; %8 ; %9 ; %10]\nMinimum Number of Points = %11\nConnectivity Epsilon = [%12 ; %13 ; %14 ; %15 ; %16]")
                               .arg(epsilon.front())
                               .arg(epsilon[epsilon.size() / 10])
                               .arg(epsilon[epsilon.size() / 2])
                               .arg(epsilon[9 * epsilon.size() / 10])
                               .arg(epsilon.back())
                               .arg(dispersion.back())
                               .arg(dispersion[9 * dispersion.size() / 10])
                               .arg(dispersion[dispersion.size() / 2])
                               .arg(dispersion[dispersion.size() / 10])
                               .arg(dispersion.front())
                               .arg(points->nb_selected_points())
                               .arg(cluster_epsilon.front())
                               .arg(cluster_epsilon[cluster_epsilon.size() / 10])
                               .arg(cluster_epsilon[cluster_epsilon.size() / 2])
                               .arg(cluster_epsilon[9 * cluster_epsilon.size() / 10])
                               .arg(cluster_epsilon.back()));
    }
}

void Polyhedron_demo_point_set_shape_detection_plugin::build_alpha_shape
(Point_set& points,  const Point_3& centroid, const Plane_3& plane,
 Scene_polyhedron_item* item, double epsilon)
{
  typedef Kernel::Point_2  Point_2;
  typedef CGAL::Alpha_shape_vertex_base_2<Kernel> Vb;
  typedef CGAL::Alpha_shape_face_base_2<Kernel>  Fb;
  typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
  typedef CGAL::Delaunay_triangulation_2<Kernel,Tds> Triangulation_2;
  typedef CGAL::Alpha_shape_2<Triangulation_2>  Alpha_shape_2;

  Vector_3 base1 = plane.base1();
  Vector_3 base2 = plane.base2();
  base1 = base1 / std::sqrt(base1 * base1);
  base2 = base2 / std::sqrt(base2 * base2);
  
  std::vector<Point_2> projections;
  projections.reserve (points.size ());

  for (Point_set::const_iterator it = points.begin(); it != points.end(); ++ it)
    projections.push_back (to_2d(centroid, base1, base2, points.point(*it)));

  Alpha_shape_2 ashape (projections.begin (), projections.end (), epsilon);
  
  std::map<Alpha_shape_2::Vertex_handle, std::size_t> map_v2i;

  Scene_polygon_soup_item *soup_item = new Scene_polygon_soup_item;
  
  soup_item->init_polygon_soup(points.size(), ashape.number_of_faces ());
  std::size_t current_index = 0;

  for (Alpha_shape_2::Finite_faces_iterator it = ashape.finite_faces_begin ();
       it != ashape.finite_faces_end (); ++ it)
    {
      if (ashape.classify (it) != Alpha_shape_2::INTERIOR)
        continue;

      for (int i = 0; i < 3; ++ i)
        {
          if (map_v2i.find (it->vertex (i)) == map_v2i.end ())
            {
              map_v2i.insert (std::make_pair (it->vertex (i), current_index ++));
              Point_3 p = to_3d (centroid, base1, base2, it->vertex (i)->point ());
              soup_item->new_vertex (p.x (), p.y (), p.z ());
            }
        }
      soup_item->new_triangle (map_v2i[it->vertex (0)],
                               map_v2i[it->vertex (1)],
                               map_v2i[it->vertex (2)]);
    }

  soup_item->orient();
  soup_item->exportAsPolyhedron (item->polyhedron());

  if (soup_item->isEmpty ())
    {
      std::cerr << "POLYGON SOUP EMPTY" << std::endl;
      for (std::size_t i = 0; i < projections.size (); ++ i)
        std::cerr << projections[i] << std::endl;
      
    }
  
  delete soup_item;
}

#include <QtPlugin>

#include "Point_set_shape_detection_plugin.moc"
