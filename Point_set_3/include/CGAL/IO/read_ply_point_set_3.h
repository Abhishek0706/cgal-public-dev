// Copyright (c) 2016  Geometry Factory
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
//
// Author(s) : Simon Giraudot

#ifndef CGAL_READ_PLY_POINT_SET_3_H
#define CGAL_READ_PLY_POINT_SET_3_H

#include <CGAL/Point_set_3.h>

namespace CGAL
{

/*!

  \ingroup PkgPointSet3

  \brief PLY interpreter designed to fill a `CGAL::Point_set_3` object.

  This interpreter will instanciate any number of property needed to
  store all PLY properties read in the header:

  - points and normals are stored in the usual `CGAL::Point_set_3`
    fashion (property "point" of type `CGAL::Point_3` and property
    "normal" of type `CGAL::Vector_3`)

  - other PLY properties are stored on point set properties with the
    name and type given by the PLY header

  \tparam Gt Geometric traits class.

  \cgalModels `PlyInterpreter`
 */

template <typename Gt>
class Ply_interpreter_point_set_3
{
  /// \cond SKIP_IN_MANUAL
public:
  typedef Point_set_3<Gt> Point_set;
  typedef Point_3<Gt> Point;
  typedef Vector_3<Gt> Vector;

private:

  struct Abstract_ply_property_to_point_set_property
  {
    virtual void assign (Ply_reader& reader, std::size_t index) = 0;
  };

  template <typename Type>
  class Ply_property_to_point_set_property : public Abstract_ply_property_to_point_set_property
  {
    typedef typename Point_set::template Property_map<Type>::type Pmap;
    Pmap m_pmap;
    std::string m_name;
  public:
    Ply_property_to_point_set_property (Point_set& ps, const std::string& name)
      : m_name (name)
    {
      bool garbage;
      boost::tie (m_pmap, garbage) = ps.add_property(name, Type());
    }
    
    virtual void assign (Ply_reader& reader, std::size_t index)
    {
      reader.assign (m_pmap[index], m_name.c_str());
    }
  };
  
  Point_set& m_point_set;
  bool m_use_floats;
  std::vector<Abstract_ply_property_to_point_set_property*> m_properties;
  /// \endcond
public:
  
  /*!
    Constructs a PLY interpreter for a `CGAL::Point_set_3` object.
   */
  Ply_interpreter_point_set_3 (Point_set& point_set)
    : m_point_set (point_set), m_use_floats (false)
  { }

  bool is_applicable (Ply_reader& reader)
  {
    const std::vector<internal::Ply_read_number*>& readers
      = reader.readers();

    bool has_normal[3] = { false, false, false };

    
    for (std::size_t i = 0; i < readers.size(); ++ i)
      {
        const std::string& name = readers[i]->name();
        if (name == "x" ||
            name == "y" ||
            name == "z")
          {
            if (dynamic_cast<internal::Ply_read_typed_number<float>*>(readers[i]))
              m_use_floats = true;
            continue;
          }
        if (name == "nx")
          {
            has_normal[0] = true;
            continue;
          }
        if (name == "ny")
          {
            has_normal[1] = true;
            continue;
          }
        if (name == "nz")
          {
            has_normal[2] = true;
            continue;
          }

        if (dynamic_cast<internal::Ply_read_typed_number<boost::int8_t>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<boost::int8_t>(m_point_set,
                                                                     name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<boost::uint8_t>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<boost::uint8_t>(m_point_set,
                                                                      name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<boost::int16_t>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<boost::int16_t>(m_point_set,
                                                                      name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<boost::uint16_t>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<boost::uint16_t>(m_point_set,
                                                                       name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<boost::int32_t>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<boost::int32_t>(m_point_set,
                                                                      name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<boost::uint32_t>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<boost::uint32_t>(m_point_set,
                                                                       name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<float>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<float>(m_point_set,
                                                             name));
          }
        else if (dynamic_cast<internal::Ply_read_typed_number<double>*>(readers[i]))
          {
            m_properties.push_back
              (new Ply_property_to_point_set_property<double>(m_point_set,
                                                             name));
          }
      }

    if (has_normal[0] && has_normal[1] && has_normal[2])
      m_point_set.add_normal_property();
   
    
    return (reader.does_tag_exist<float> ("x") || reader.does_tag_exist<double> ("x"))
      && (reader.does_tag_exist<float> ("y") || reader.does_tag_exist<double> ("y"))
      && (reader.does_tag_exist<float> ("z") || reader.does_tag_exist<double> ("z"));
  }
  
  void process_line (Ply_reader& reader)
  {
    m_point_set.add_item();
    
    if (m_use_floats)
      process_line<float>(reader);
    else
      process_line<double>(reader);

    for (std::size_t i = 0; i < m_properties.size(); ++ i)
      m_properties[i]->assign (reader, m_point_set.size() - 1);
  }

  /// \cond SKIP_IN_MANUAL
  template <typename FT>
  void process_line (Ply_reader& reader)
  {
    FT x = (FT)0.,y = (FT)0., z = (FT)0.,
      nx = (FT)0., ny = (FT)0., nz = (FT)0.;
    reader.assign (x, "x");
    reader.assign (y, "y");
    reader.assign (z, "z");
    Point point (x, y, z);
    m_point_set.point(m_point_set.size() - 1) = point;

    if (m_point_set.has_normals())
      {
        reader.assign (nx, "nx");
        reader.assign (ny, "ny");
        reader.assign (nz, "nz");
        Vector normal (nx, ny, nz);
        m_point_set.normal(m_point_set.size() - 1) = normal;
      }
  }
  /// \endcond
};

}

#endif // CGAL_READ_PLY_POINT_SET_3_H
