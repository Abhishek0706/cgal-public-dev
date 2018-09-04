// Copyright (c) 2017 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0+
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//
#ifndef CGAL_PATH_ON_SURFACE_H
#define CGAL_PATH_ON_SURFACE_H 1

#include <stack>
#include <CGAL/Union_find.h>
#include <CGAL/Path_generators.h>
#include <boost/unordered_map.hpp>

namespace CGAL {

template<typename Map_>
class Path_on_surface
{
public:
  typedef Map_ Map;
  typedef typename Map::Dart_handle Dart_handle;
  typedef typename Map::Dart_const_handle Dart_const_handle;

  typedef Path_on_surface<Map> Self;

  Path_on_surface(const Map& amap) : m_map(amap), m_is_closed(false)
  {}

  void swap(Self& p2)
  {
    assert(&m_map==&(p2.m_map));
    m_path.swap(p2.m_path);
    std::swap(m_is_closed, p2.m_is_closed);
  }

  /// @Return true if this path is equal to other path, identifying dart 0 of
  ///          this path with dart start in other path.
  bool are_same_paths_from(const Self& other, std::size_t start) const
  {
    assert(start==0 || start<length());

    if (length()!=other.length() || is_closed()!=other.is_closed())
    { return false; }

    if (!is_closed() && start>0) { return false; }

    for(std::size_t i=0; i<length(); ++i)
    {
      if (get_ith_dart(i)!=other.get_ith_dart(start))
      { return false; }
      start=next_index(start);
    }
    return true;
  }

  /// @return true if this path is equal to other path. For closed paths, test
  ///         all possible starting darts.
  bool operator==(const Self& other) const
  {
    if (length()!=other.length() || is_closed()!=other.is_closed())
    { return false; }

    if (!is_closed())
    { return are_same_paths_from(other, 0); }

    for(std::size_t start=0; start<length(); ++start)
    {
      if (are_same_paths_from(other, start))
      { return true; }
    }
    return false;
  }
  bool operator!=(const Self&  other) const
  { return !(operator==(other)); }

  /// @Return true if this path is equal to other path, identifying dart 0 of
  ///          this path with dart start in other path. other path is given
  ///          by index of its darts, in text format.
  bool are_same_paths_from(const char* other, std::size_t start) const
  {
    assert(start==0 || start<length());

    std::string sother(other);
    std::istringstream iss(sother);
    uint64_t nb;

    if (!is_closed() && start>0) { return false; }

    for(std::size_t i=0; i<length(); ++i)
    {
      if (!iss.good())
      { return false; }
      iss>>nb;
      if (nb!=m_map.darts().index(get_ith_dart(start)))
      { return false; }
      start=next_index(start);
    }
    iss>>nb;
    if (iss.good())
    { return false; } // There are more elements in other than in this path

    return true;
  }

  /// @return true if this path is equal to other path. For closed paths, test
  ///         all possible starting darts. other path is given by index of its
  ///         darts, in text format.
  bool operator==(const char*  other) const
  {
    if (!is_closed())
    { return are_same_paths_from(other, 0); }

    for(std::size_t start=0; start<length(); ++start)
    {
      if (are_same_paths_from(other, start))
      { return true; }
    }
    return false;
  }
  bool operator!=(const char*  other) const
  { return !(operator==(other)); }

  // @return true iff the path is empty
  bool is_empty() const
  { return m_path.empty(); }

  std::size_t length() const
  { return m_path.size(); }

  // @return true iff the path is closed (update after each path modification).
  bool is_closed() const
  { return m_is_closed; }

  const Map& get_map() const
  { return m_map; }

  void clear()
  {
    m_path.clear();
    m_is_closed=false;
  }
  
  void cut(std::size_t n, bool update_isclosed=true)
  {
    if (n>=length()) return;
    m_path.resize(n);
    if (update_isclosed) { update_is_closed(); }
  }

  std::size_t next_index(std::size_t i) const
  { return (is_closed() && i==m_path.size()-1?0:i+1); }

  std::size_t prev_index(std::size_t i) const
  { return (is_closed() && i==0?m_path.size()-1:i-1); }

  Dart_const_handle get_ith_dart(std::size_t i) const
  {
    assert(i<m_path.size());
    return m_path[i];
  }
  
  Dart_const_handle operator[] (std::size_t i) const
  { return get_ith_dart(i); }

  Dart_const_handle get_prev_dart(std::size_t i) const
  {
    assert(i<m_path.size());
    if (i==0 && !is_closed()) return NULL;
    return m_path[prev_index(i)];
  }

  Dart_const_handle get_next_dart(std::size_t i) const
  {
    assert(i<m_path.size());
    if (i==m_path.size()-1 && !is_closed()) return NULL;
    return m_path[next_index(i)];
  }

  Dart_const_handle back() const
  {
    assert(!is_empty());
    return m_path.back();
  }
  
  void push_back(Dart_const_handle dh, bool update_isclosed=true)
  {
    assert(dh!=NULL && dh!=m_map.null_dart_handle);
    /* This assert is too long...
     assert((is_empty() ||
           CGAL::template belong_to_same_cell<Map, 0>
           (m_map, m_map.other_extremity(back()), dh))); */

    m_path.push_back(dh);
    if (update_isclosed) { update_is_closed(); }
  }

  // @return true iff the path is valid; i.e. a sequence of edges two by
  //              two adjacent.
  bool is_valid() const
  {
    for (unsigned int i=1; i<m_path.size(); ++i)
    {
      if (!m_map.darts().owns(m_path[i]))
      { return false; }

      if (m_path[i]==NULL || m_path[i]==m_map.null_dart_handle)
      { return false; }

      Dart_const_handle pend=m_map.other_extremity(m_path[i-1]);
      if (pend==Map::null_handle) { return false; }

      if (!CGAL::template belong_to_same_cell<Map,0>(m_map, m_path[i], pend))
      { return false; }
    }
    if (is_closed())
    {
      Dart_const_handle pend=m_map.other_extremity(m_path[m_path.size()-1]);
      if (pend==Map::null_handle) { return false; }
      if (!CGAL::template belong_to_same_cell<Map,0>(m_map, pend, m_path[0]))
      { return false; }
    }

    return true;
  }

  // Update m_is_closed to true iff the path is closed (i.e. the second
  //   extremity of the last dart of the path is the same vertex than the one
  //   of the first dart of the path).
  void update_is_closed()
  {
    if (is_empty()) { m_is_closed=false; } // or true by vacuity ?
    else if (!is_valid()) { m_is_closed=false; } // Interest ??
    else
    {
      Dart_const_handle pend=m_map.other_extremity(back());
      if (pend==Map::null_handle) { m_is_closed=false; }
      else 
      { m_is_closed=CGAL::belong_to_same_cell<Map,0>(m_map, m_path[0], pend); }
    }
  }

  // @return true iff the path does not pass twice through a same edge
  //              or a same vertex.
  bool is_simple() const
  {
    typename Map::size_type markvertex=m_map.get_new_mark();
    typename Map::size_type markedge=m_map.get_new_mark();

    bool res=true;
    unsigned int i=0;
    for (i=0; res && i<m_path.size(); ++i)
    {
      if (m_map.is_marked(m_path[i], markvertex)) res=false;
      if (m_map.is_marked(m_path[i], markedge)) res=false;

      CGAL::mark_cell<Map, 0>(m_path[i], markvertex);
      CGAL::mark_cell<Map, 1>(m_path[i], markedge);
    }

    i=0;
    while(m_map.number_of_marked_darts(markedge)>0)
    {
      assert(i<m_path.size());
      CGAL::unmark_cell<Map, 0>(m_path[i], markvertex);
      CGAL::unmark_cell<Map, 1>(m_path[i], markedge);
      ++i;
    }

    m_map.free_mark(markvertex);
    m_map.free_mark(markedge);

    return res;
  }

  void reverse()
  {
    std::vector<Dart_const_handle> new_path(m_path.size());
    for (std::size_t i=0; i<m_path.size(); ++i)
    {
      new_path[m_path.size()-1-i]=m_map.template beta<2>(m_path[i]);
    }
    new_path.swap(m_path);
  }

  /// If the given path is opened, close it by doing the same path that the
  /// first one in reverse direction.
  void close()
  {
    if (!is_closed())
    {
      for (int i=m_path.size()-1; i>=0; --i)
      { m_path.push_back(m_map.template beta<2>(get_ith_dart(i))); }
      m_is_closed=true;
    }
  }

  /// @return the turn between dart number i and dart number i+1.
  ///         (turn is position of the second edge in the cyclic ordering of
  ///          edges starting from the first edge around the second extremity
  ///          of the first dart)
  std::size_t next_positive_turn(std::size_t i) const
  {
    assert(is_valid());
    assert(i<m_path.size());

    if (!is_closed() && i==length()-1)
    { return 0; }

    Dart_const_handle d1=get_ith_dart(i);
    Dart_const_handle d2=get_next_dart(i);
    assert(d1!=d2);

    std::size_t res=1;
    while (m_map.template beta<1>(d1)!=d2)
    {
      ++res;
      d1=m_map.template beta<1, 2>(d1);
    }
    // std::cout<<"next_positive_turn="<<res<<std::endl;
    return res;
  }

  /// Same than next_positive_turn but turning in reverse orientation around vertex.
  std::size_t next_negative_turn(std::size_t i) const
  {
    assert(is_valid());
    assert(i<m_path.size());

    if (!is_closed() && i==length()-1)
    { return 0; }

    Dart_const_handle d1=m_map.template beta<2>(get_ith_dart(i));
    Dart_const_handle d2=m_map.template beta<2>(get_next_dart(i));
    assert(d1!=d2);

    std::size_t res=1;
    while (m_map.template beta<0>(d1)!=d2)
    {
      ++res;
      d1=m_map.template beta<0, 2>(d1);
    }
    // std::cout<<"next_negative_turn="<<res<<std::endl;
    return res;
  }

  std::size_t find_end_of_braket(std::size_t begin, bool positive) const
  {
    assert((positive && next_positive_turn(begin)==1) ||
           (!positive && next_negative_turn(begin)==1));
    std::size_t end=next_index(begin);
    if (!is_closed() && end>=length()-1)
    { return begin; } // begin is the before last dart

    while ((positive && next_positive_turn(end)==2) ||
           (!positive && next_negative_turn(end)==2))
    { end=next_index(end); }
    
    if ((positive && next_positive_turn(end)==1) ||
        (!positive && next_negative_turn(end)==1)) // We are on the end of a bracket
    { end=next_index(end); }
    else
    { end=begin; }
    
    return end;
  }

  void transform_positive_bracket(std::size_t begin, std::size_t end,
                                  Self& new_path)
  {
    // There is a special case for (1 2^r). In this case, we need to ignore
    // the two darts begin and end
    Dart_const_handle d1=(next_index(begin)!=end?
          m_map.template beta<0>(get_ith_dart(begin)):
          m_map.template beta<1,2,0>(get_ith_dart(end)));
    Dart_const_handle d2=(next_index(begin)!=end?
          m_map.template beta<2,0,2>(get_ith_dart(end)):
          m_map.template beta<0,0,2>(get_ith_dart(begin)));

    new_path.push_back(m_map.template beta<2>(d1));
    CGAL::extend_straight_negative_until(new_path, d2);
  }

  void transform_negative_bracket(std::size_t begin, std::size_t end,
                                  Self& new_path)
  {
    // There is a special case for (-1 -2^r). In this case, we need to ignore
    // the two darts begin and end
    Dart_const_handle d1=(next_index(begin)!=end?
          m_map.template beta<2,1>(get_ith_dart(begin)):
          m_map.template beta<2,0,2,1>(get_ith_dart(end)));
    Dart_const_handle d2=(next_index(begin)!=end?
          m_map.template beta<1>(get_ith_dart(end)):
          m_map.template beta<2,1,1>(get_ith_dart(begin)));

    new_path.push_back(d1);
    CGAL::extend_straight_positive_until(new_path, d2);
  }

  void transform_bracket(std::size_t begin, std::size_t end,
                         Self& new_path,
                         bool positive)
  {
    if (positive)
    { transform_positive_bracket(begin, end, new_path); }
    else
    { transform_negative_bracket(begin, end, new_path); }
  }

  // copy all darts starting from begin and going to the dart before end
  // from this path to new_path.
  void copy_rest_of_path(std::size_t begin, std::size_t end,
                         Self& new_path)
  {
    assert(end<=length());
    assert(begin<=end);
    while(begin!=end)
    {
      new_path.push_back(get_ith_dart(begin));
      ++begin;
    }
  }

  bool bracket_flattening_one_step()
  {
    if (is_empty()) return false;

    Self new_path(m_map);
    bool positive=false;
    std::size_t begin, end;
    std::size_t lastturn=m_path.size()-(is_closed()?0:1);

    for (begin=0; begin<lastturn; ++begin)
    {
      positive=(next_positive_turn(begin)==1);
      if (positive || next_negative_turn(begin)==1)
      {
        // we test if begin is the beginning of a bracket
        end=find_end_of_braket(begin, positive);
        if (begin!=end)
        {
          /* std::cout<<"Bracket: ["<<begin<<"; "<<end<<"] "
                   <<(positive?"+":"-")<<std::endl; */
          if (end<begin)
          {
            if (!is_closed())
            { return false; }

            copy_rest_of_path(end+1, begin, new_path);
          }
          else if (next_index(begin)!=end) // Special case of (1 2^r)
          { copy_rest_of_path(0, begin, new_path); }

          transform_bracket(begin, end, new_path, positive);

          if (begin<end && next_index(begin)!=end && end<length()-1)
          { copy_rest_of_path(end+1, length(), new_path); }

          swap(new_path);
          return true;
        }
      }
    }

    return false;
  }

  // Simplify the path by removing all brackets
  bool bracket_flattening()
  {
    bool res=false;
    while(bracket_flattening_one_step())
    { res=true; }
    return res;
  }
  
  bool remove_spurs_one_step()
  {
    if (is_empty()) return false;

    bool res=false;
    std::size_t i;
    std::size_t lastturn=m_path.size()-(is_closed()?0:1);
    Self new_path(m_map);
    for (i=0; i<lastturn; )
    {
      if (m_path[i]==m_map.template beta<2>(m_path[next_index(i)]))
      {
        i+=2;
        res=true;
      }
      else
      {
        new_path.push_back(m_path[i]); // We copy this dart
        ++i;
      }
    }
    if (i==m_path.size()-1)
    { new_path.push_back(m_path[m_path.size()-1]); } // we copy the last dart

    swap(new_path);
    return res;
  }

  // Simplify the path by removing all spurs
  bool remove_spurs()
  {
    bool res=false;
    while(remove_spurs_one_step())
    { res=true; }
    return res;
  }

  // Simplify the path by removing all possible brackets and spurs
  void simplify()
  {
    bool modified=false;
    do
    {
      modified=bracket_flattening_one_step();
      if (!modified)
      { modified=remove_spurs_one_step(); }
    }
    while(modified);
  }

  bool find_l_shape(std::size_t begin,
                    std::size_t& middle,
                    std::size_t& end) const
  {
    assert(next_negative_turn(begin)==1 || next_negative_turn(begin)==2);
    end=begin+1;
    if (end==m_path.size()-1 && !is_closed())
    { return false; } // begin is the before last dart

    while (next_negative_turn(end)==2 && end!=begin)
    { end=next_index(end); }

    if (begin==end)
    { // Case of a path having only 2 turns
      return true;
    }

    if (next_negative_turn(end)==1)
    {
      middle=end;
      end=next_index(end);
    }
    else
    { return false; }

    while (next_negative_turn(end)==2 && end!=begin)
    { end=next_index(end); }

    return true;
  }

  void push_l_shape(std::size_t begin,
                    std::size_t middle,
                    std::size_t end,
                    Self& new_path,
                    bool case_seven)
  {
    Dart_const_handle d1;

    if (!case_seven)
    { d1=m_map.template beta<2,1>(get_ith_dart(begin)); }
    else
    { d1=m_map.template beta<2,1,2,0>(get_ith_dart(begin)); }
    new_path.push_back(d1);

    if (begin!=middle)
    {
      if (!case_seven)
      { CGAL::extend_uturn_positive(new_path, 1); }

      d1=m_map.template beta<2,1,1>(get_ith_dart(middle));
      CGAL::extend_straight_positive_until(new_path, d1);

      if (next_index(middle)!=end)
      { CGAL::extend_uturn_positive(new_path, 3); }
      else
      { CGAL::extend_straight_positive(new_path, 1); }
    }

    if (next_index(middle)!=end)
    {
      d1=m_map.template beta<2,0,2,1>(get_ith_dart(end));
      CGAL::extend_straight_positive_until(new_path, d1);

      if (!case_seven)
      { CGAL::extend_uturn_positive(new_path, 1); }
      else
      { CGAL::extend_straight_positive(new_path, 1); }
    }

    if (begin==middle && next_index(middle)==end)
    { // TODO: check if we need to do also something for !case_seven ?
      // if (case_seven)
      { CGAL::extend_uturn_positive(new_path, 1); }
      /* else
      { assert(false); } // We think (?) that this case is not possible */
    }
  }

  void push_l_shape_cycle_2()
  {
    Dart_const_handle d1=
        m_map.template beta<2,1,1>(get_ith_dart(0));
    clear();
    push_back(d1);
    CGAL::extend_straight_positive(*this, 1);
    CGAL::extend_straight_positive_until(*this, d1);
  }

  bool push_l_shape_2darts()
  {
    Dart_const_handle d1=NULL;

    if (next_negative_turn(0)==1)
      d1=m_map.template beta<2,1>(get_ith_dart(0));
    else if (next_negative_turn(1)==1)
      d1=m_map.template beta<2,1>(get_ith_dart(1));
    else return false;

    clear();
    push_back(d1);
    CGAL::extend_uturn_positive(*this, 1);
    //push_back(m_map.template beta<1>(d1));
    return true;
  }

  bool right_push_one_step()
  {
    if (is_empty()) { return false; }

    if (length()==2)
    { return push_l_shape_2darts(); }

    std::size_t begin, middle, end;
    std::size_t lastturn=m_path.size()-(is_closed()?0:1);
    std::size_t next_turn;
    std::size_t val_x=0; // value of turn before the beginning of a l-shape
    bool prev2=false;

    for (middle=0; middle<lastturn; ++middle)
    {
      next_turn=next_negative_turn(middle);

      if (next_turn==2)
      {
        if (!prev2)
        {
          begin=middle; // First 2 of a serie
          prev2=true;
          if (begin==0 && is_closed())
          {
            begin=length()-1;
            do
            {
              next_turn=next_negative_turn(begin);
              if (next_turn==2) { --begin; }
              if (begin==0) // Loop of only -2 turns
              {
                push_l_shape_cycle_2();
                return true;
              }
            }
            while(next_turn==2);
            begin=next_index(begin); // because we stopped on a dart s.t. next_turn!=2
          }
          // Here begin is the first dart of the path s.t. next_turn==-2
          // i.e. the previous turn != -2
        }
      }
      else
      {
        if (next_turn==1)
        {
          // Here middle is a real middle; we already know begin (or we know
          // that there is no -2 before if !prev2), we only need to compute
          // end.
          if (!prev2) { begin=middle; } // There is no -2 before this -1
          end=next_index(middle);
          do
          {
            next_turn=next_negative_turn(end);
            if (next_turn==2) { end=next_index(end); }
            assert(end!=middle);
          }
          while(next_turn==2);

          if (is_closed() || begin>0)
          { val_x=next_negative_turn(prev_index(begin)); }

          // And here now we can push the path
          Self new_path(m_map);
          if (end<begin)
          {
            if (!is_closed())
            { return false; }

            copy_rest_of_path(end+1, begin, new_path);
          }
          else
          { copy_rest_of_path(0, begin, new_path); }

          // std::cout<<prev_index(begin)<<"  "<<next_index(end)<<std::endl;
          bool case_seven=(val_x==3 && prev_index(begin)==end);

          push_l_shape(begin, middle, end, new_path, case_seven);

          if (begin<end)
          { copy_rest_of_path(end+1, length(), new_path); }

          swap(new_path);
          return true;

        }
        prev2=false;
      }
    }
    return false;
  }

  bool right_push()
  {
    bool res=false;
    while(right_push_one_step())
    { res=true;
      
      std::cout<<"RP "; display();  display_pos_and_neg_turns();
      std::cout<<std::endl;
    }
    return res;
  }

  // Canonize the path
  void canonize()
  {
    if (!is_closed())
    { return; }

    std::cout<<"##########################################"<<std::endl;
    std::cout<<"Init "; display();
    std::cout<<std::endl;
    display_pos_and_neg_turns();
    std::cout<<std::endl;

    bool modified=false;
    std::cout<<"RS ";
    remove_spurs_one_step();

    display(); display_pos_and_neg_turns();
    std::cout<<std::endl;

    do
    {
      do
      {
        modified=bracket_flattening_one_step();

        std::cout<<"BF "; display(); display_pos_and_neg_turns();
        std::cout<<std::endl;

        modified=modified || remove_spurs_one_step();

        std::cout<<"RS "; display(); display_pos_and_neg_turns();
        std::cout<<std::endl;
      }
      while(modified);

      modified=right_push();
    }
    while(modified); // Maybe we do not need to iterate, a unique last righ_push should be enough (? To verify)

  }

  std::vector<std::size_t> compute_positive_turns() const
  {
    std::vector<std::size_t> res;
    if (is_empty()) return res;

    std::size_t i;
    for (i=0; i<m_path.size()-1; ++i)
    {
      if (m_path[i]==m_map.template beta<2>(m_path[i+1]))
      { res.push_back(0); }
      else { res.push_back(next_positive_turn(i)); }
    }
    if (is_closed())
    {
      if (m_path[i]==m_map.template beta<2>(m_path[0]))
      { res.push_back(0); }
      else { res.push_back(next_positive_turn(i)); }
    }
    return res;
  }

  std::vector<std::size_t> compute_negative_turns() const
  {
    std::vector<std::size_t> res;
    if (is_empty()) return res;

    std::size_t i;
    for (i=0; i<m_path.size()-1; ++i)
    {
      if (m_path[i]==m_map.template beta<2>(m_path[i+1]))
      { res.push_back(0); }
      else { res.push_back(next_negative_turn(i)); }
    }
    if (is_closed())
    {
      if (m_path[i]==m_map.template beta<2>(m_path[0]))
      { res.push_back(0); }
      else { res.push_back(next_negative_turn(i)); }
    }
    return res;
  }

  std::vector<std::size_t> compute_turns(bool positive) const
  { return (positive?compute_positive_turns():compute_negative_turns()); }

  bool same_turns_from(const char* turns,
                       const std::vector<std::size_t>& resplus,
                       const std::vector<std::size_t>& resmoins,
                       std::size_t start) const
  {
    assert(start==0 || start<resplus.size());
    assert(resplus.size()==resmoins.size());

    std::string sturns(turns);
    std::istringstream iss(sturns);
    int64_t nb;

    for(std::size_t i=0; i<resplus.size(); ++i)
    {
      if (!iss.good())
      { return false; }
      iss>>nb;
      if ((nb>=0 && resplus[start]!=nb) ||
          (nb<0 && resmoins[start]!=-nb))
      { return false; }

      start=next_index(start);
    }
    iss>>nb;
    if (iss.good())
    { return false; } // There are more elements in turns than in res

    return true;
  }

  bool same_turns(const char* turns) const
  {
    std::vector<std::size_t> resplus=compute_positive_turns();
    std::vector<std::size_t> resmoins=compute_negative_turns();

    if (!is_closed())
    { return same_turns_from(turns, resplus, resmoins, 0); }

    for (std::size_t start=0; start<length(); ++start)
    {
      if (same_turns_from(turns, resplus, resmoins, start))
      { return true; }
    }

    return false;
  }

  void display_positive_turns() const
  {
    std::cout<<"+(";
    std::vector<std::size_t> res=compute_positive_turns();
    for (std::size_t i=0; i<res.size(); ++i)
    { std::cout<<res[i]<<(i<res.size()-1?" ":""); }
    std::cout<<")";
  }

  void display_negative_turns() const
  {
    std::cout<<"-(";
    std::vector<std::size_t> res=compute_negative_turns();
    for (std::size_t i=0; i<res.size(); ++i)
    { std::cout<<res[i]<<(i<res.size()-1?" ":""); }
    std::cout<<")";
  }

  void display_pos_and_neg_turns() const
  {
    display_positive_turns();
    std::cout<<"  ";
    display_negative_turns();
  }

  void display() const
  {
    for (std::size_t i=0; i<length(); ++i)
    {
      std::cout<<m_map.darts().index(get_ith_dart(i));
      if (i<length()-1) { std::cout<<" "; }
    }
     if (is_closed())
     { std::cout<<" c "; } //<<m_map.darts().index(get_ith_dart(0)); }
  }

protected:
  const Map& m_map; // The underlying map
  std::vector<Dart_const_handle> m_path; // The sequence of darts
  bool m_is_closed; // True iff the path is a cycle
};

} // namespace CGAL

#endif // CGAL_PATH_ON_SURFACE_H //
// EOF //
