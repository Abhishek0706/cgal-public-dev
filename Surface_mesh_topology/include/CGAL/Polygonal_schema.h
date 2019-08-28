// Copyright (c) 2019 CNRS and LIRIS' Establishments (France).
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
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef CGAL_POLYGONAL_SCHEMA_H
#define CGAL_POLYGONAL_SCHEMA_H 1

#include <CGAL/license/Surface_mesh_topology.h>

#include <vector>
#include <unordered_map>
#include <cstddef>
#include <string>
#include <initializer_list>
#include <CGAL/assertions.h>
#include <CGAL/memory.h>
#include <CGAL/Polygonal_schema_min_items.h>
#include <CGAL/Combinatorial_map.h>
#include <CGAL/Generalized_map.h>

namespace CGAL {
namespace Surface_mesh_topology {

  namespace internal 
  {
    /// @return opposite label of label s
    ///    (i.e. add/remove - depending if s is positive or negative)
    inline std::string opposite_label(const std::string & s)
    {
      CGAL_assertion(!s.empty());
      if (s[0]=='-')
      { return s.substr(1, std::string::npos); }
      
      return std::string("-")+s;
    }

    template<class Map, class Combinatorial_data_structure=
             typename Map::Combinatorial_data_structure>
    struct Polygonal_schema_tools
    {};
    template<class CMap>
    struct Polygonal_schema_tools<CMap, Combinatorial_map_tag>
    {
      typedef typename CMap::Dart_handle Dart_handle;
      
      static Dart_handle
      add_edge_to_face(CMap& cmap, const std::string& s,
                       Dart_handle prev_dart,
                       Dart_handle dart_same_label,
                       Dart_handle dart_opposite_label,
                       std::unordered_map<std::string, Dart_handle>& edge_label_to_dart)
      {
        if (dart_same_label!=nullptr && dart_opposite_label!=nullptr)
        {
          std::cerr<<"Polygonal_schema ERROR: "<<"both labels "<<s
                   <<" and "<<internal::opposite_label(s)<<" are already added in the surface."
                   <<" This label can not be use anymore."<<std::endl;
          return nullptr;
        }
        
        if (dart_same_label!=nullptr)
        {
          std::cerr<<"Polygonal_schema ERROR: "<<"label "<<s
                   <<" is already added in the surface."
                   <<" Since the surface is orientable, this label can not be use anymore."<<std::endl;
          return nullptr;
        }
        
        Dart_handle res=cmap.create_dart();
        edge_label_to_dart[s]=res;
        
        cmap.info(res).m_label=new char[s.size()+1];
        strncpy(cmap.info(res).m_label, s.c_str(), s.size()+1); // +1 to copy also the \0 char
        
        if (prev_dart!=cmap.null_handle)
        { cmap.template link_beta<1>(prev_dart, res); }
        
        if (dart_opposite_label!=nullptr)
        { cmap.template link_beta<2>(res, dart_opposite_label); }
        
        return res;
      }
      
      std::string get_label(CMap& cmap, Dart_handle dh) const
      {
        CGAL_assertion(cmap.info(dh).m_label!=nullptr);
        return std::string(cmap.info(dh).m_label);
      }
    };
    template<class GMap>
    struct Polygonal_schema_tools<GMap, Generalized_map_tag>
    {
      typedef typename GMap::Dart_handle Dart_handle;
      
      // In a GMap, if an edge is 2-free, only one of its two dart has one label.
      // Otherwise, d has one label and alpha<0,2>(d) the opposite label.
      static Dart_handle
      add_edge_to_face(GMap& gmap, const std::string& s,
                       Dart_handle prev_dart,
                       Dart_handle dart_same_label,
                       Dart_handle dart_opposite_label,
                       std::unordered_map<std::string, Dart_handle>& edge_label_to_dart)
      {
        if (dart_same_label!=nullptr && dart_opposite_label!=nullptr)
        {
          std::cerr<<"Polygonal_schema ERROR: "<<"both labels "<<s
                   <<" and "<<internal::opposite_label(s)<<" are already added in the surface."
                   <<" This label can not be use anymore."<<std::endl;
          return nullptr;
        }
      
        Dart_handle res=gmap.create_dart();
        Dart_handle dh2=gmap.create_dart();

        gmap.template link_alpha<0>(res, dh2);
        if (prev_dart!=gmap.null_handle)
        { gmap.template link_alpha<1>(res, gmap.template alpha<0>(prev_dart)); }

        if (dart_same_label!=nullptr)
        { // Here dart_same_label!=nullptr
          std::string s2=internal::opposite_label(s);
          edge_label_to_dart[s2]=dh2;
          gmap.info(dh2).m_label=new char[s2.size()+1];
          strncpy(gmap.info(dh2).m_label, s2.c_str(), s2.size()+1); // +1 to copy also the \0 char

          gmap.template sew<2>(res, dart_same_label);
        }
        else
        { // Here either dart_opposite_label!=nullptr, or both are nullptr
          edge_label_to_dart[s]=res;
          gmap.info(res).m_label=new char[s.size()+1];
          strncpy(gmap.info(res).m_label, s.c_str(), s.size()+1); // +1 to copy also the \0 char

          if (dart_opposite_label!=nullptr)
          {
            std::string s2=internal::opposite_label(s);
            edge_label_to_dart[s2]=res;
            gmap.info(res).m_label=new char[s2.size()+1];
            strncpy(gmap.info(res).m_label, s2.c_str(), s2.size()+1); // +1 to copy also the \0 char
          
            gmap.template sew<2>(dh2, dart_opposite_label);
          }
        }

        return res;
      }

      std::string get_label(GMap& gmap, Dart_handle dh) const
      {
        char* label=gmap.info(dh).m_label;

        if (label==nullptr)
        {
          if (!gmap.is_free<2>(dh))
          { label=gmap.info(gmap.template alpha<2>(dh)).m_label; }
          else
          {
            return internal::opposite_label
              (std::string(gmap.info(gmap.template alpha<0>(dh))));
          }
        }
        CGAL_assertion(label!=nullptr);
        return std::string(label);
      }
    };

  }
  // end namespace internal

  struct Combinatorial_map_tag;
  struct Generalized_map_tag;
  
  template < class BaseModel >
  class Polygonal_schema_base: public BaseModel
  {
  public:
    typedef BaseModel                       Base;
    typedef Polygonal_schema_base           Self;
    typedef BaseModel                       Map; // Either a GMap or a CMap
    typedef typename Map::Dart_handle       Dart_handle;
    typedef typename Map::Dart_const_handle Dart_const_handle;
    typedef typename Map::size_type         size_type;

    Polygonal_schema_base() : Base(),
      first_dart(this->null_handle),
      prev_dart(this->null_handle),
      facet_started(false),
      mark_border(this->get_new_mark())
    {}

    ~Polygonal_schema_base()
    {
      for (auto it=this->darts().begin(), itend=this->darts().end(); it!=itend; ++it)
      {
        if (this->info(it).m_label!=nullptr)
        {
          delete []this->info(it).m_label;
          this->info(it).m_label=nullptr;
        }
      }
    }
    
    /// Start a new facet.
    void begin_facet()
    {
      if (facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to start a facet"
                 <<" but the previous facet is not yet ended."<<std::endl;
        return;
      }

      first_dart = this->null_handle;
      prev_dart  = this->null_handle;
      facet_started=true;
    }

    /// Add one edge to the current facet, given by its label (any string, using minus sign for orientation)
    void add_edge_to_facet(const std::string& s)
    {
      if (!facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to add an edge to a facet"
                 <<" but the facet is not yet started."<<std::endl;
        return;
      }

      Dart_handle dart_same_label=get_dart_labeled(s);
      Dart_handle dart_opposite_label=get_dart_labeled(internal::opposite_label(s));
      
      Dart_handle cur=internal::Polygonal_schema_tools<Map>::
        add_edge_to_face(*this, s, prev_dart, dart_same_label, dart_opposite_label,
                         edge_label_to_dart);

      if (prev_dart==this->null_handle)
      { first_dart=cur; }

      prev_dart=cur;
    }

    /// add the given edges to the current facet
    /// s is a sequence of labels, add all the corresponding edges into the current facet.
    void add_edges_to_facet(const std::string& s)
    {
      if (!facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to add edges to a facet"
                 <<" but the facet is not yet started."<<std::endl;
        return;
      }
      std::istringstream iss(s);
      for (std::string token; std::getline(iss, token, ' '); )
      { add_edge_to_facet(token); }
    }
      
    /// add one facet, s is a sequence of labels, add all the corresponding edges into a new facet.
    void add_facet(const std::string& s)
    {
      if (facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to add a new facet"
                 <<" but the previous facet is not yet ended."<<std::endl;
        return;
      }
      begin_facet();
      add_edges_to_facet(s);
      end_facet();
    }

    void add_edges_to_facet(std::initializer_list<const char*> l)
    {
      if (!facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to add edges to a facet"
                 <<" but the facet is not yet started."<<std::endl;
        return;
      }
       for (const char* e : l)
       { add_edge_to_facet(e); }
    }
      
    void add_facet(std::initializer_list<const char*> l)
    {
      if (facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to add a new facet"
                 <<" but the previous facet is not yet ended."<<std::endl;
        return;
      }
      begin_facet();
      add_edges_to_facet(l);
      end_facet();
    }
    
    /// End of the facet. Return the first dart of this facet.
    Dart_handle end_facet()
    {
      if (!facet_started)
      {
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to end a facet"
                 <<" but the facet is not yet started."<<std::endl;
        return nullptr;
      }
      CGAL_assertion( first_dart!=this->null_handle && prev_dart!=this->null_handle );
      this->set_next(prev_dart, first_dart);
      
      facet_started=false;
      return first_dart;
    }

    /// @return dart with the given label, nullptr if this dart does not exist.
    Dart_handle get_dart_labeled(const std::string & s) const
    {
      auto ite=edge_label_to_dart.find(s);
      if (ite==edge_label_to_dart.end())
      { return nullptr; }

      return ite->second;
    }

    std::string get_label(Dart_handle dh) const
    { return internal::Polygonal_schema_tools<Map>::get_label(dh); }

    /// marks the whole facet containing dh as a border
    /// @return the number of darts of the marked face
    unsigned int set_facet_to_border(Dart_handle dh)
    {
      if (this->is_marked(dh, mark_border))
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to set a facet to border"
                 <<" but the facet is already a border."<<std::endl;
        return 0;
      }
      
      return CGAL::mark_cell<Self, 2>(*this, dh, mark_border);
    }

    /// same fonciton but using a label
    unsigned int set_facet_to_border(const std::string & s)
    {
      auto ite=edge_label_to_dart.find(s);
      if (ite==edge_label_to_dart.end())
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to label "<<s<<" to be a border"
                 <<" but this label does not exist yet."<<std::endl;
        return 0;
      }

      return set_facet_to_border(ite->second);
    }

    /// remove the border marks from the whole facet
    /// @return the number of darts of the unmarked face
    unsigned int set_facet_to_non_border(Dart_handle dh)
    {
      if (!this->is_marked(dh, mark_border))
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to set a facet to non border"
                 <<" but the facet is already a non border."<<std::endl;
        return 0;
      }
      
      unsigned res=0;
      for (auto it=this->template darts_of_cell<2>(dh).begin(),
             itend=this->template darts_of_cell<2>(dh).end();
               it!=itend; ++it)
      {
        this->unmark(it, mark_border);
        ++res;
      }
      return res;
    }

    /// same fonciton but using a label
    unsigned int set_facet_to_non_border(const std::string & s)
    {
      auto ite=edge_label_to_dart.find(s);
      if (ite==edge_label_to_dart.end())
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you try to label "<<s<<" to be a non border"
                 <<" but this label does not exist yet."<<std::endl;
        return 0;
      }

      return set_facet_to_non_border(ite->second);
    }

    /// @return true iff dh is a border
    bool is_border(Dart_handle dh)
    {
      return this->is_marked(dh, mark_border);
    }

    /// same thing but using a label instead of a dart
    bool is_border(const std::string & s)
    {
      auto ite=edge_label_to_dart.find(s);
      if (ite==edge_label_to_dart.end())
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you ask if label "<<s<<" represents a dart border"
                 <<" but this label does not exist yet."<<std::endl;
        return false;
      }

      return is_border(ite->second);
    }

    /// @return true iff dh is i-free
    template<unsigned int i>
    bool is_free(Dart_const_handle dh) const
    {
      if(i==2)
      {
        return Base::template is_free<2>(dh)
            || this->is_marked(this->template beta<2>(dh), mark_border);
      }
      return Base::template is_free<i>(dh);
    }

    /// same thing but using a label instead of a dart
    template<unsigned int i>
    bool is_free(const std::string & s) const
    {
      auto ite=edge_label_to_dart.find(s);
      if (ite==edge_label_to_dart.end())
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you ask if label "<<s<<" represents a dart border"
                 <<" but this label does not exist yet."<<std::endl;
        return false;
      }

      return Self::template is_free<i>(ite->second);
    }

    /// Non templated versions
    /// @return true iff dh is i-free
    bool is_free(Dart_const_handle dh, unsigned int i) const
    {
      if(i==2)
      {
        return Base::is_free(dh, 2)
            || this->is_marked(this->template beta<2>(dh), mark_border);
      }
      return Base::is_free(dh, i);
    }

    /// same thing but using a label instead of a dart
    bool is_free(const std::string & s, unsigned int i) const
    {
      auto ite=edge_label_to_dart.find(s);
      if (ite==edge_label_to_dart.end())
      {// maybe there is no need to put an error message, someone may want to set a facet to border without knowing if it is already the case
        std::cerr<<"Polygonal_schema ERROR: "
                 <<"you ask if label "<<s<<" represents a dart border"
                 <<" but this label does not exist yet."<<std::endl;
        return false;
      }

      return is_free(ite->second, i);
    }

    void display_borders()
    {
      std::cout<<"labels is_free<2> is_border"<<std::endl;
      for (auto it=edge_label_to_dart.begin(), itend=edge_label_to_dart.end(); it!=itend; ++it)
      {
        std::cout<<it->first<<" "<<Self::template is_free<2>(it->second)<<" "<<is_border(it->second)<<std::endl;
      }
    }
    
  protected:
    // For each edge label, its corresponding dart. Stores both association a -a, to allow
    // users to start to add either a or -a.
    std::unordered_map<std::string, Dart_handle> edge_label_to_dart;
    
    Dart_handle first_dart;
    Dart_handle prev_dart;
    bool        facet_started;
    std::size_t mark_border;
  };

  template <class Items_=Polygonal_schema_min_items,
            class Alloc_=CGAL_ALLOCATOR(int),
            class Storage_= Combinatorial_map_storage_1<2, Items_, Alloc_> >
  class Polygonal_schema_with_combinatorial_map:
    public Polygonal_schema_base<CGAL::Combinatorial_map_base
                                 <2,
                                  Polygonal_schema_with_combinatorial_map<Items_, Alloc_, Storage_>,
                                  Items_, Alloc_, Storage_> >
  {
  public:
    typedef Polygonal_schema_with_combinatorial_map<Items_, Alloc_, Storage_>  Self;
    typedef Combinatorial_map_base<2, Self, Items_, Alloc_, Storage_>          CMap_base;
    typedef Polygonal_schema_base<CMap_base>                                   Base;
    
    typedef typename Base::Dart_handle Dart_handle;
    typedef typename Base::Dart_const_handle Dart_const_handle;

    Polygonal_schema_with_combinatorial_map() : Base()
    {}

    Polygonal_schema_with_combinatorial_map(const Self & amap) : Base(amap)
    {}

    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2>
    Polygonal_schema_with_combinatorial_map(const Combinatorial_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap) : Base(amap)
    {}

    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2, typename Converters>
    Polygonal_schema_with_combinatorial_map(const Combinatorial_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap, const Converters& converters) :
      Base(amap, converters)
    {}
    
    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2, typename Converters,
              typename DartInfoConverter>
    Polygonal_schema_with_combinatorial_map(const Combinatorial_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap, const Converters& converters,
                                            const DartInfoConverter& dartinfoconverter) :
      Base(amap, converters, dartinfoconverter)
    {}

    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2, typename Converters,
              typename DartInfoConverter, typename PointConverter >
    Polygonal_schema_with_combinatorial_map(const Combinatorial_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap, const Converters& converters,
                                            const DartInfoConverter& dartinfoconverter,
                                            const PointConverter& pointconverter) :
      Base(amap, converters, dartinfoconverter, pointconverter)
    {}
  };
  
  template <class Items_=Polygonal_schema_min_items,
            class Alloc_=CGAL_ALLOCATOR(int),
            class Storage_= Generalized_map_storage_1<2, Items_, Alloc_> >
  class Polygonal_schema_with_generalized_map:
    public Polygonal_schema_base<CGAL::Generalized_map_base
                                 <2,
                                  Polygonal_schema_with_generalized_map<Items_, Alloc_, Storage_>,
                                  Items_, Alloc_, Storage_> >
  {
  public:
    typedef Polygonal_schema_with_generalized_map<Items_, Alloc_, Storage_> Self;
    typedef Generalized_map_base<2, Self, Items_, Alloc_, Storage_>         GMap_base;
    typedef Polygonal_schema_base<GMap_base>                                Base;
    
    typedef typename Base::Dart_handle Dart_handle;
    typedef typename Base::Dart_const_handle Dart_const_handle;

    Polygonal_schema_with_generalized_map() : Base()
    {}

    Polygonal_schema_with_generalized_map(const Self & amap) : Base(amap)
    {}

    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2>
    Polygonal_schema_with_generalized_map(const Generalized_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap) : Base(amap)
    {}

    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2, typename Converters>
    Polygonal_schema_with_generalized_map(const Generalized_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap, const Converters& converters) :
      Base(amap, converters)
    {}
    
    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2, typename Converters,
              typename DartInfoConverter>
    Polygonal_schema_with_generalized_map(const Generalized_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap, const Converters& converters,
                                            const DartInfoConverter& dartinfoconverter) :
      Base(amap, converters, dartinfoconverter)
    {}

    template <unsigned int d2, typename Refs2, typename Items2, typename Alloc2,
              typename Storage2, typename Converters,
              typename DartInfoConverter, typename PointConverter >
    Polygonal_schema_with_generalized_map(const Generalized_map_base<d2, Refs2, Items2, Alloc2, Storage2>&
                                            amap, const Converters& converters,
                                            const DartInfoConverter& dartinfoconverter,
                                            const PointConverter& pointconverter) :
      Base(amap, converters, dartinfoconverter, pointconverter)
    {}
  };
  
} //namespace Surface_mesh_topology
} //namespace CGAL

#endif // CGAL_POLYGONAL_SCHEMA_H //
// EOF //
