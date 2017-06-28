#ifndef HANDLES_H
#define HANDLES_H

#include"typedefs.h"
#include<vector>
#include<unordered_map>
#include<map>
//#include"hexextr.h"

//class HexExtr;
class Vertex_handle{
  public:
      Vertex_handle(LCC_3 &lcc, Dart_handle &dh, int e, bool f){
      incident_dart = dh;
      current_point = lcc.point(dh);
      next_point = lcc.point(lcc.alpha(dh, 0));
      enumeration = e;
      boundary = f;
    }
    Dart_handle incident_dart;
    Point current_point;
    Point next_point;
    int enumeration;
    bool boundary;
};

class Edge_handle{
  public:
    Edge_handle(LCC_3 &lcc, Dart_handle &dh){
      associated_dart = dh;
      from_point = lcc.point(dh);
      to_point = lcc.point(lcc.alpha(dh, 0));
      //next_edge = 
    }
    Dart_handle associated_dart;
    Point from_point;
    Point to_point;
    int enumeration;
    //Edge_handle next_edge;
};


class Face_handle{
  public:
    Face_handle(LCC_3 &lcc, Dart_handle& dh, int i){
      a_dart = dh;
      for(LCC_3::Dart_of_cell_range<3>:: iterator it=lcc.darts_of_cell<3>(dh).begin(), 
itend=lcc.darts_of_cell<3>(dh).end(); it!=itend; ++it){
        incident_darts.push_back(it);
      }
      enumeration = i;
    }
    std::vector<Dart_handle> incident_darts;
    Dart_handle a_dart;
    int enumeration;
    
    bool operator==(const Face_handle &other) const{ 
      return (enumeration == other.enumeration);
    }
};



/*
class Cell_handle{

};
*/
#endif
