// Copyright (c) 2019 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is a part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
//
// Author(s)     : Dmitry Anisimov, Simon Giraudot, Pierre Alliez, Florent Lafarge, and Andreas Fabri
//

#ifndef CGAL_LEVELS_OF_DETAIL_INTERNAL_BUILDINGS_SITE_H
#define CGAL_LEVELS_OF_DETAIL_INTERNAL_BUILDINGS_SITE_H

#include <CGAL/license/Levels_of_detail.h>

// STL includes.
#include <memory>
#include <vector>
#include <utility>

// Boost includes.
#include <boost/optional/optional.hpp>

// CGAL includes.
#include <CGAL/property_map.h>
#include <CGAL/Optimal_transportation_reconstruction_2.h>

// LOD includes.
#include <CGAL/Levels_of_detail/enum.h>

// Internal includes.
#include <CGAL/Levels_of_detail/internal/utils.h>
#include <CGAL/Levels_of_detail/internal/struct.h>

// Simplification.
#include <CGAL/Levels_of_detail/internal/Simplification/Thinning_2.h>
#include <CGAL/Levels_of_detail/internal/Simplification/Grid_based_filtering_2.h>
#include <CGAL/Levels_of_detail/internal/Simplification/Alpha_shapes_filtering_2.h>
#include <CGAL/Levels_of_detail/internal/Simplification/Generic_simplifier.h>

// Regularization.
#include <CGAL/Levels_of_detail/internal/Regularization/Regularization.h>

// Spatial search.
#include <CGAL/Levels_of_detail/internal/Spatial_search/K_neighbor_query.h>
#include <CGAL/Levels_of_detail/internal/Spatial_search/Sphere_neighbor_query.h>
#include <CGAL/Levels_of_detail/internal/Spatial_search/Stored_neighbor_query.h>

// Shape detection.
#include <CGAL/Levels_of_detail/internal/Shape_detection/Region_growing.h>
#include <CGAL/Levels_of_detail/internal/Shape_detection/Estimate_normals_2.h>
#include <CGAL/Levels_of_detail/internal/Shape_detection/Least_squares_line_fit_region.h>
#include <CGAL/Levels_of_detail/internal/Shape_detection/Least_squares_line_fit_sorting.h>
#include <CGAL/Levels_of_detail/internal/Shape_detection/Visibility_based_region.h>

// Partitioning.
#include <CGAL/Levels_of_detail/internal/Partitioning/Partition_builder_2.h>
#include <CGAL/Levels_of_detail/internal/Partitioning/Kinetic_partitioning_2.h>

// Visibility.
#include <CGAL/Levels_of_detail/internal/Visibility/Visibility_2.h>

// Graphcut.
#include <CGAL/Levels_of_detail/internal/Graphcut/Graphcut.h>

// Buildings.
#include <CGAL/Levels_of_detail/internal/Buildings/Building_builder.h>
#include <CGAL/Levels_of_detail/internal/Buildings/Building_roofs.h>

// Experimental.
#include <CGAL/Levels_of_detail/internal/Experimental/Visibility_stable_2.h>

namespace CGAL {
namespace Levels_of_detail {
namespace internal {

  template<typename DataStructure>
  class Buildings_site {

  public:
    using Data_structure = DataStructure;

    using Traits = typename Data_structure::Traits;
    using Point_map = typename Data_structure::Point_map;

    using FT = typename Traits::FT;
    using Point_2 = typename Traits::Point_2;
    using Point_3 = typename Traits::Point_3;
    using Vector_2 = typename Traits::Vector_2;
    using Vector_3 = typename Traits::Vector_3;
    using Plane_3 = typename Traits::Plane_3;
    using Segment_2 = typename Traits::Segment_2;
    using Segment_3 = typename Traits::Segment_3;
    using Line_2 = typename Traits::Line_2;
    using Triangle_2 = typename Traits::Triangle_2;

    using Points = std::vector<std::size_t>;
    using Point_map_2 = typename Data_structure::Point_map_2;
    using Point_map_3 = typename Data_structure::Point_map_3;
    using Visibility_map_d = typename Data_structure::Visibility_map_d;

    using Building = internal::Building<Traits>;
    using Building_ptr = std::shared_ptr<Building>;
    
    using Indexer = internal::Indexer<Point_3>;
    
    using Points_2 = std::vector<Point_2>;
    using Vectors_2 = std::vector<Vector_2>;
    using Pair_item_2 = std::pair<Point_2, Vector_2>;
    using Pair_range_2 = std::vector<Pair_item_2>;
    using Identity_map = CGAL::Identity_property_map<Point_2>;
    using First_of_pair_map = CGAL::First_of_pair_property_map<Pair_item_2>;
    using Second_of_pair_map = CGAL::Second_of_pair_property_map<Pair_item_2>;
    using Boundary_point_map_2 = 
    internal::Item_property_map<Points_2, Identity_map>;
    
    using K_neighbor_query =
    internal::K_neighbor_query<Traits, Points_2, Identity_map>;
    using Sphere_neighbor_query =
    internal::Sphere_neighbor_query<Traits, Points_2, Identity_map>;

    using Grid_based_filtering_2 = internal::Grid_based_filtering_2<Traits>;
    using Alpha_shapes_filtering_2 = internal::Alpha_shapes_filtering_2<Traits>;
    using Thinning_2 = internal::Thinning_2<Traits, Sphere_neighbor_query>;

    using Normal_estimator_2 = 
    internal::Estimate_normals_2<Traits, Points_2, Identity_map, Sphere_neighbor_query>;
    using LSLF_region = 
    internal::Least_squares_line_fit_region<Traits, Pair_range_2, First_of_pair_map, Second_of_pair_map>;
    using LSLF_sorting =
    internal::Least_squares_line_fit_sorting<Traits, Points_2, Sphere_neighbor_query, Identity_map>;
    using Region_growing_2 = 
    internal::Region_growing<Points_2, Sphere_neighbor_query, LSLF_region, typename LSLF_sorting::Seed_map>;

    using Partition_2 = internal::Partition_2<Traits>;
    using Partition_builder_2 = internal::Partition_builder_2<Traits>;
    using Kinetic_partitioning_2 = internal::Kinetic_partitioning_2<Traits>;
    using Graphcut_2 = internal::Graphcut<Traits, Partition_2>;

    using Partition_faces_2 = std::vector<typename Partition_2::Face>;
    using Stored_neighbor_query = 
    internal::Stored_neighbor_query<Traits, Partition_faces_2>;
    using Visibility_based_region = 
    internal::Visibility_based_region<Traits, Partition_faces_2>;
    using Partition_region_growing_2 = 
    internal::Region_growing<Partition_faces_2, Stored_neighbor_query, Visibility_based_region>;

    using Building_builder = internal::Building_builder<Traits, Partition_2, Points, Point_map_3>;
    using Building_roofs = internal::Building_roofs<Data_structure>;

    using Generic_simplifier = internal::Generic_simplifier<Traits, Point_map_3>;
    using Regularization = internal::Regularization<Traits>;

    Buildings_site(
      const Data_structure& data,
      const Points& interior_points,
      const Points& boundary_points,
      const std::size_t site_index) : 
    m_data(data),
    m_interior_points(interior_points),
    m_boundary_points(boundary_points),
    m_site_index(site_index),
    m_boundaries_detected(false),
    m_footprints_computed(false),
    m_footprints_extruded(false),
    m_roofs_detected(false),
    m_roofs_computed(false) { 

      CGAL_precondition(m_interior_points.size() > 0);
      CGAL_precondition(m_boundary_points.size() >= 0);
      
      m_all_points.clear();
      m_all_points.reserve(m_interior_points.size() + m_boundary_points.size());
      for (const std::size_t idx : m_interior_points)
        m_all_points.push_back(idx);
      for (const std::size_t idx : m_boundary_points)
        m_all_points.push_back(idx);

      create_ground_plane();
    }

    void detect_boundaries() {

      // const FT sampling_2 = m_data.parameters.buildings.grid_cell_width_2;
      // const FT thinning_2 = m_data.parameters.scale / FT(2);

      /*
      extract_boundary_points_2(
        m_data.parameters.buildings.alpha_shape_size_2,
        sampling_2, 
        m_data.parameters.buildings.grid_cell_width_2,
        thinning_2); */

      extract_boundary_points_2(
        m_data.parameters.buildings.grid_cell_width_2,
        m_data.parameters.buildings.alpha_shape_size_2,
        m_data.parameters.buildings.imagecut_beta_2);

      extract_wall_points_2(
        m_data.parameters.buildings.region_growing_scale_2,
        m_data.parameters.buildings.region_growing_noise_level_2,
        m_data.parameters.buildings.region_growing_angle_2,
        m_data.parameters.buildings.region_growing_min_length_2);
      compute_approximate_boundaries();

      /*
      compute_optimal_transport(
        m_data.parameters.scale,
        m_data.parameters.noise_level); */
      
      regularize_segments(
        m_data.parameters.buildings.regularization_angle_bound_2,
        m_data.parameters.buildings.regularization_ordinate_bound_2);
    }

    void compute_footprints() {

      partition_2(
        m_data.parameters.buildings.kinetic_min_face_width_2, 
        m_data.parameters.buildings.kinetic_max_intersections_2);

      compute_visibility_2();

      /*
      partition_2();
      compute_visibility_2(
        m_data.parameters.buildings.alpha_shape_size_2); */

      apply_graphcut_2(
        m_data.parameters.buildings.graphcut_beta_2);
      
      initialize_buildings();

      compute_building_footprints(
        m_data.parameters.buildings.min_faces_per_footprint);
    }

    void extrude_footprints() {
      extrude_building_footprints(
        m_data.parameters.buildings.extrusion_type);
    }

    void detect_roofs() {
      if (!m_footprints_extruded)
        return;
      if (m_buildings.empty())
        return;

      exit(EXIT_SUCCESS);

      m_building_roofs.clear();
      m_building_roofs.reserve(m_buildings.size());
      for (std::size_t i = 0; i < m_buildings.size(); ++i) {
        const auto& cluster = 
          m_building_interior_clusters[m_buildings[i].cluster_index];
        m_building_roofs.push_back(
          Building_roofs(
            m_data, 
            cluster,
            m_buildings[i]));
      }

      for (auto& building_roofs : m_building_roofs)
        building_roofs.detect_roofs();

      m_roofs_detected = true;
    }

    void compute_roofs() {
      if (!m_roofs_detected)
        return;
      if (m_building_roofs.empty())
        return;

      for (auto& building_roofs : m_building_roofs)
        building_roofs.compute_roofs();

      for (auto& building_roofs : m_building_roofs)
        if (building_roofs.empty()) 
          building_roofs.set_flat_roofs();

      m_roofs_computed = true;
    }

    const Plane_3& ground_plane() const {
      return m_ground_plane;
    }

    void get_buildings(
      std::vector<Building_ptr>& buildings) const {
      
      buildings.clear();
      if (m_buildings.empty()) return;
      for (const auto& building : m_buildings)
        buildings.push_back(std::make_shared<Building>(building));
    }

    template<typename OutputIterator>
    boost::optional<OutputIterator> 
    get_boundary_points(
      OutputIterator output) const {

      if (m_boundary_points_2.empty())
        return boost::none;
        
      for (const Point_2& p : m_boundary_points_2) {
        const Point_3 q = internal::position_on_plane_3(p, m_ground_plane);
        *(output++) = std::make_pair(q, std::size_t(-1));
      }
      return output;
    }

    template<typename OutputIterator>
    boost::optional<OutputIterator> 
    get_wall_points(
      OutputIterator output) const {

      if (m_wall_points_2.empty())
        return boost::none;
      
      Identity_map identity_map;
      Boundary_point_map_2 point_map_2(m_boundary_points_2, identity_map);

      for (std::size_t i = 0; i < m_wall_points_2.size(); ++i) {
        for (const std::size_t idx : m_wall_points_2[i]) {
          const Point_2& p = get(point_map_2, idx);
          const Point_3 q = internal::position_on_plane_3(p, m_ground_plane);
          *(output++) = std::make_pair(q, i);
        }
      }
      return output;
    }

    template<typename OutputIterator>
    boost::optional<OutputIterator> 
    get_approximate_boundaries(
      OutputIterator output) const {
      
      if (m_approximate_boundaries_2.empty())
        return boost::none;
        
      for (std::size_t i = 0; i < m_approximate_boundaries_2.size(); ++i) {
        const Point_2& s = m_approximate_boundaries_2[i].source();
        const Point_2& t = m_approximate_boundaries_2[i].target();

        const Point_3 p = internal::position_on_plane_3(s, m_ground_plane);
        const Point_3 q = internal::position_on_plane_3(t, m_ground_plane);
        *(output++) = std::make_pair(Segment_3(p, q), i);
      }
      return output;
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_partitioning_2(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces, 
      const FT z) const {
      
      if (m_partition_2.empty())
        return boost::none;

      for (const auto& face : m_partition_2.faces)
        face.output_for_visibility(
          indexer, num_vertices, vertices, faces, z);
      return std::make_pair(vertices, faces);
    } 

    template<typename OutputIterator>
    boost::optional<OutputIterator> 
    get_building_points(
      OutputIterator output,
      std::size_t& building_index) const {
      
      if (m_building_interior_clusters.empty() &&
          m_building_boundary_clusters.empty())
        return boost::none;

      CGAL_assertion(
        m_building_interior_clusters.size() == m_building_boundary_clusters.size());
      for (std::size_t i = 0; i < m_building_interior_clusters.size(); ++i) {
        for (const std::size_t pidx : m_building_interior_clusters[i])
          *(output++) = std::make_pair(get(m_data.point_map_3, pidx), building_index);
        for (const std::size_t pidx : m_building_boundary_clusters[i])
          *(output++) = std::make_pair(get(m_data.point_map_3, pidx), building_index);
        ++building_index;
      }
      return output;
    }

    template<typename OutputIterator>
    boost::optional<OutputIterator> 
    get_building_boundaries(
      OutputIterator output,
      std::size_t& building_index) const {
      
      if (m_buildings.empty())
        return boost::none;

      for (const auto& building : m_buildings) {
        for (const auto& edge : building.edges0) {
          const Point_2& s = edge.segment.source();
          const Point_2& t = edge.segment.target();
          const FT z = edge.z;
          *(output++) = std::make_pair(
            Segment_3(Point_3(s.x(), s.y(), z), Point_3(t.x(), t.y(), z)), 
            building_index);
        }
        ++building_index;
      }
      return output;
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_building_footprints(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {
      
      if (m_buildings.empty())
        return boost::none;

      for (const auto& building : m_buildings) {
        building.base0.output_for_object(
          indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_extruded_building_boundaries(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {
      
      if (m_buildings.empty())
        return boost::none;

      for (const auto& building : m_buildings) {
        for (const auto& wall : building.walls1)
          wall.output_for_object(
            indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_extruded_building_footprints(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {
      
      if (m_buildings.empty())
        return boost::none;

      for (const auto& building : m_buildings) {
        for (const auto& roof : building.roofs1)
          roof.output_for_object(
            indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

    template<typename OutputIterator>
    boost::optional<OutputIterator> 
    get_roof_points(
      OutputIterator output) const {

      if (m_building_roofs.empty())
        return boost::none;

      std::size_t roof_index = 0;
      for (const auto& building_roofs : m_building_roofs)
        building_roofs.get_roof_points(output, roof_index);
      return output;
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_building_approximate_bounds(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {
      
      if (m_building_roofs.empty())
        return boost::none;

      for (const auto& building_roofs : m_building_roofs) {
        building_roofs.get_approximate_bounds(
          indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_building_partitioning_3(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {

      if (m_building_roofs.empty())
        return boost::none;

      for (const auto& building_roofs : m_building_roofs) {
        building_roofs.get_partitioning_3(
          indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_building_walls(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {
      
      if (m_building_roofs.empty())
        return boost::none;

      for (const auto& building_roofs : m_building_roofs) {
        building_roofs.get_walls_corresponding_to_roofs(
          indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

    template<
    typename VerticesOutputIterator,
    typename FacesOutputIterator>
    boost::optional< std::pair<VerticesOutputIterator, FacesOutputIterator> > 
    get_building_roofs(
      Indexer& indexer,
      std::size_t& num_vertices,
      VerticesOutputIterator vertices,
      FacesOutputIterator faces,
      std::size_t& building_index) const {
      
      if (m_building_roofs.empty())
        return boost::none;

      for (const auto& building_roofs : m_building_roofs) {
        building_roofs.get_roofs(
          indexer, num_vertices, vertices, faces, building_index);
        ++building_index;
      }
      return std::make_pair(vertices, faces);
    }

  private:
    const Data_structure& m_data;
    const Points& m_interior_points;
    const Points& m_boundary_points;
    const std::size_t m_site_index;
    Plane_3 m_ground_plane;

    Points m_all_points;
    std::vector<Building> m_buildings;
    Points_2 m_boundary_points_2;
    std::vector< std::vector<std::size_t> > m_wall_points_2;
    std::vector<Segment_2> m_approximate_boundaries_2;
    Partition_2 m_partition_2;
    std::vector< std::vector<std::size_t> > m_building_bases_2;
    std::vector<Points> m_building_interior_clusters;
    std::vector<Points> m_building_boundary_clusters;

    bool m_boundaries_detected;
    bool m_footprints_computed;
    bool m_footprints_extruded;
    bool m_roofs_detected;
    bool m_roofs_computed;

    std::vector<Building_roofs> m_building_roofs;
    std::shared_ptr<Generic_simplifier> m_simplifier_ptr;

    void create_ground_plane() {

      FT minz = internal::max_value<FT>();
      for (const std::size_t idx : m_interior_points)
        minz = CGAL::min(minz, get(m_data.point_map_3, idx).z());
      for (const std::size_t idx : m_boundary_points)
        minz = CGAL::min(minz, get(m_data.point_map_3, idx).z());
      m_ground_plane = 
      Plane_3(Point_3(FT(0), FT(0), minz), Vector_3(FT(0), FT(0), FT(1)));
    }

    void extract_boundary_points_2(
      const FT alpha_shape_size_2, 
      const FT sampling_2,
      const FT grid_cell_width_2,
      const FT thinning_2) {

      m_boundary_points_2.clear();
      apply_alpha_shapes_filtering_2(alpha_shape_size_2, sampling_2);
      apply_grid_based_filtering_2(grid_cell_width_2);
      apply_thinning_2(thinning_2);
    }

    void extract_boundary_points_2(
      const FT grid_cell_width_2,
      const FT alpha_shape_size_2,
      const FT imagecut_beta_2) {
      
      m_boundary_points_2.clear();

      m_simplifier_ptr = std::make_shared<Generic_simplifier>(
        m_all_points, 
        m_data.point_map_3,
        grid_cell_width_2,
        alpha_shape_size_2,
        imagecut_beta_2);

      m_simplifier_ptr->create_cluster();
      m_simplifier_ptr->transform_cluster();
      m_simplifier_ptr->create_grid();
      m_simplifier_ptr->create_image();
      m_simplifier_ptr->get_outer_boundary_points_2(m_boundary_points_2);
    }

    void apply_alpha_shapes_filtering_2(
      const FT alpha_shape_size_2, 
      const FT sampling_2) {

      const std::size_t numi = m_interior_points.size();
      const std::size_t numb = m_boundary_points.size();

      if (numi < 3 && numb < 3) return;
      Alpha_shapes_filtering_2 filtering(alpha_shape_size_2);

      if (numi >= 3)
        filtering.add_points(m_interior_points, m_data.point_map_3);
      filtering.get_filtered_points(sampling_2, m_boundary_points_2);
      if (numb >= 3)
        filtering.add_points(m_boundary_points, m_data.point_map_3);
      filtering.get_filtered_points(sampling_2, m_boundary_points_2);
    }

    void apply_grid_based_filtering_2(
      const FT grid_cell_width_2) {

      if (m_boundary_points_2.empty()) return;
      const Grid_based_filtering_2 filtering(grid_cell_width_2);
			filtering.apply(m_boundary_points_2);
    }

    void apply_thinning_2(
      const FT scale) {

      Identity_map identity_map;
      Sphere_neighbor_query neighbor_query(
        m_boundary_points_2, scale, identity_map);
      const Thinning_2 thinning(neighbor_query);
      thinning.apply(m_boundary_points_2);
    }

    void extract_wall_points_2(
      const FT region_growing_scale_2,
      const FT region_growing_noise_level_2,
      const FT region_growing_angle_2,
      const FT region_growing_min_length_2) {

      if (m_boundary_points_2.empty())
        return;

      m_wall_points_2.clear();
      
      Identity_map identity_map;
      Sphere_neighbor_query neighbor_query(
        m_boundary_points_2, region_growing_scale_2, identity_map);

      Vectors_2 normals;
      Normal_estimator_2 estimator(
        m_boundary_points_2, neighbor_query, identity_map);
      estimator.get_normals(normals);

      CGAL_assertion(m_boundary_points_2.size() == normals.size());
      Pair_range_2 range;
      range.reserve(m_boundary_points_2.size());
      for (std::size_t i = 0; i < m_boundary_points_2.size(); ++i)
        range.push_back(std::make_pair(m_boundary_points_2[i], normals[i]));

      First_of_pair_map point_map;
      Second_of_pair_map normal_map;
      LSLF_region region(
        range, 
        region_growing_noise_level_2,
        region_growing_angle_2,
        region_growing_min_length_2,
        point_map,
        normal_map);

      LSLF_sorting sorting(
        m_boundary_points_2, neighbor_query, identity_map);
      sorting.sort();

      Region_growing_2 region_growing(
        m_boundary_points_2,
        neighbor_query,
        region,
        sorting.seed_map());
      region_growing.detect(std::back_inserter(m_wall_points_2));
    }

    void compute_approximate_boundaries() {

      if (m_wall_points_2.empty())
        return;

      m_approximate_boundaries_2.clear();
      m_approximate_boundaries_2.reserve(m_wall_points_2.size());

      Identity_map identity_map;
      Boundary_point_map_2 point_map_2(
        m_boundary_points_2, identity_map);

      Line_2 line; Point_2 p, q; 
      std::vector<std::size_t> indices;
      for (const auto& item_range : m_wall_points_2) {
        indices.clear();
        for (std::size_t i = 0; i < item_range.size(); ++i)
          indices.push_back(i);
        internal::line_from_points_2(
          item_range, point_map_2, line);
        internal::boundary_points_on_line_2(
          item_range, point_map_2, indices, line, p, q);
        m_approximate_boundaries_2.push_back(Segment_2(p, q));
      }
      CGAL_assertion(
        m_approximate_boundaries_2.size() == m_wall_points_2.size());
      m_boundaries_detected = true;
    }

    void compute_optimal_transport(
      const FT scale, const FT noise_level) {

      if (m_boundary_points_2.empty())
        return;

      // Split boundary points into connected components.
      using CC_neighbor_query =
      internal::Sphere_neighbor_query<Traits, Points_2, Identity_map>;
      using CC_region = 
      internal::Connected_component_region;
      using CC_region_growing = 
      internal::Region_growing<Points_2, CC_neighbor_query, CC_region>;

      Identity_map identity_map;
      CC_neighbor_query neighbor_query(
        m_boundary_points_2, scale, identity_map);
      CC_region region_type(2);
      CC_region_growing region_growing(
        m_boundary_points_2, neighbor_query, region_type);
      
      m_wall_points_2.clear();
      region_growing.detect(std::back_inserter(m_wall_points_2));
      
      std::cout << "OTR num regions: " << m_wall_points_2.size() << std::endl;

      // Apply otr to each component separately.
      using PMap_2 = typename CC_neighbor_query::Index_to_point_map;
      using Otr = CGAL::Optimal_transportation_reconstruction_2<Traits, PMap_2>;
      
      m_approximate_boundaries_2.clear();
      for (const auto& region : m_wall_points_2) {
        
        Otr otr(region, neighbor_query.point_map());
        otr.run_under_wasserstein_tolerance(noise_level);
        otr.list_output(
          boost::make_function_output_iterator([&](const Point_2&) -> void { }),
          boost::make_function_output_iterator([&](const Segment_2& segment_2) -> void {
            m_approximate_boundaries_2.push_back(segment_2);
          })
        );
      }
      m_boundaries_detected = true;
    }

    void regularize_segments(
      const FT regularization_angle_bound_2,
      const FT regularization_ordinate_bound_2) {
      
      CGAL_assertion(m_approximate_boundaries_2.size() >= 2);
      Regularization regularization(
        m_approximate_boundaries_2);
      regularization.regularize_angles(regularization_angle_bound_2);
      regularization.regularize_ordinates(regularization_ordinate_bound_2);
    }

    void partition_2(
      const FT kinetic_min_face_width_2,
      const std::size_t kinetic_max_intersections_2) {

      if (!m_boundaries_detected) return;
      if (m_approximate_boundaries_2.empty()) return;
      
			const Kinetic_partitioning_2 kinetic(
        kinetic_min_face_width_2,
        kinetic_max_intersections_2);
			kinetic.compute(
        m_approximate_boundaries_2,
        m_partition_2);
    }

    void partition_2() {

      if (!m_boundaries_detected) return;
      if (m_approximate_boundaries_2.empty()) return;

      const Partition_builder_2 partition_builder(
        m_approximate_boundaries_2);
      partition_builder.build(m_partition_2);
    }

    void compute_visibility_2(const FT alpha_shape_size_2) {
      
      if (!m_boundaries_detected) return;
      if (m_partition_2.empty()) return;

      using PV_pair = std::pair<Point_2, bool>;
      std::vector<PV_pair> reference_points;
      m_simplifier_ptr->get_regular_points(reference_points);
      
      using PMap = CGAL::First_of_pair_property_map<PV_pair>;

      PMap pmap;

      using Visibility_2 = internal::Visibility_stable_2<Traits, std::vector<PV_pair>, PMap>;
      const Visibility_2 visibility(
        reference_points, pmap, alpha_shape_size_2);
      visibility.compute(m_partition_2);
    }

    void compute_visibility_2() {
      
      if (!m_boundaries_detected) return;
      if (m_partition_2.empty()) return;

      using PV_pair = std::pair<Point_2, bool>;
      std::vector<PV_pair> reference_points;
      m_simplifier_ptr->get_regular_points(reference_points);

      using PMap = CGAL::First_of_pair_property_map<PV_pair>;
      using VMap = CGAL::Second_of_pair_property_map<PV_pair>;
      
      PMap pmap; VMap vmap;

      using Visibility_2 = internal::Visibility_2<Traits, std::vector<PV_pair>, PMap, VMap>;
      Visibility_2 visibility(reference_points, pmap, vmap);
      visibility.compute(m_partition_2);
    }

    void apply_graphcut_2(
      const FT graphcut_beta_2) {

      if (!m_boundaries_detected) return;
      if (m_partition_2.empty()) return;

      const Graphcut_2 graphcut(graphcut_beta_2);
      graphcut.apply(m_partition_2);
    }

    void initialize_buildings() {

      if (!m_boundaries_detected) return;
      if (m_partition_2.empty()) return;

      m_building_bases_2.clear();
      const auto& faces = m_partition_2.faces;

      // Region growing.
      Stored_neighbor_query neighbor_query(faces);
      Visibility_based_region region(faces);
      Partition_region_growing_2 region_growing(
        faces, neighbor_query, region);
      region_growing.detect(
        std::back_inserter(m_building_bases_2));

      // Clustering.
      CGAL_assertion(m_interior_points.size() > 0);

      m_building_interior_clusters.clear();
      m_building_interior_clusters.resize(m_building_bases_2.size());
      m_building_boundary_clusters.clear();
      m_building_boundary_clusters.resize(m_building_bases_2.size());

      for (std::size_t i = 0; i < m_building_bases_2.size(); ++i) {
        add_cluster_points(
          m_interior_points, m_building_bases_2[i], faces, 
          m_building_interior_clusters[i]);
        add_cluster_points(
          m_boundary_points, m_building_bases_2[i], faces, 
          m_building_boundary_clusters[i]);
      }
    }

    void add_cluster_points(
      const Points& points,
      const std::vector<std::size_t>& findices,
      const std::vector<typename Partition_2::Face>& faces,
      Points& cluster) const {

      if (points.empty())
        return;

      for (const std::size_t pidx : points) {
        const Point_2& p = get(m_data.point_map_2, pidx);
        for (const std::size_t fidx : findices) {
          const auto& tri = faces[fidx].base.delaunay;
          
          bool found = false;
          for (auto fh = tri.finite_faces_begin(); 
          fh != tri.finite_faces_end(); ++fh) {
           
            const Triangle_2 triangle = Triangle_2(
              fh->vertex(0)->point(),
              fh->vertex(1)->point(),
              fh->vertex(2)->point());
           
            if (internal::is_within_triangle_2(
              p, triangle, m_data.parameters.noise_level * FT(2))) {
              
              cluster.push_back(pidx);
              found = true;
              break;
            }
          }
          if (found) break;
        }
      }
    }

    void compute_building_footprints(
      const std::size_t min_faces_per_footprint) {

      if (!m_boundaries_detected) return;
      m_buildings.clear();
      CGAL_assertion(m_building_bases_2.size() == m_building_interior_clusters.size());
      CGAL_assertion(m_building_bases_2.size() == m_building_boundary_clusters.size());
      const Building_builder builder(m_partition_2);
      
      std::size_t idx = 0; Building building;
      for (std::size_t i = 0; i < m_building_bases_2.size(); ++i) {
        building.cluster_index = i;
        building.index = idx;
        
        if (m_building_boundary_clusters[building.cluster_index].empty())
          builder.add_lod0(
            m_building_bases_2[i], 
            m_building_interior_clusters[building.cluster_index], 
            m_data.point_map_3,
            building);
        else 
          builder.add_lod0(
            m_building_bases_2[i], 
            m_building_boundary_clusters[building.cluster_index], 
            m_data.point_map_3,
            building);
        
        if (building.base0.triangulation.delaunay.number_of_faces() 
          >= min_faces_per_footprint) {  
          m_buildings.push_back(building);
          ++idx;
        }
      }
      m_footprints_computed = true;
    }

    void extrude_building_footprints(
      const Extrusion_type extrusion_type) {
      
      if (!m_footprints_computed) return;
      const Building_builder builder(m_partition_2);

      for (std::size_t i = 0; i < m_buildings.size(); ++i) 
        builder.add_lod1( 
          extrusion_type,
          m_building_interior_clusters[m_buildings[i].cluster_index], 
          m_data.point_map_3,
          m_buildings[i]);
      m_footprints_extruded = true;
    }
  };

} // internal
} // Levels_of_detail
} // CGAL

#endif // CGAL_LEVELS_OF_DETAIL_INTERNAL_BUILDINGS_SITE_H
