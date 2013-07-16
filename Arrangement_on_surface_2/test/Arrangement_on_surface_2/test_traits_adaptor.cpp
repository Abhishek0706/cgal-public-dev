#include <CGAL/basic.h>
#include "test_configuration.h"
#include <iostream>

#include <CGAL/assertions.h>
#include <CGAL/Arrangement_2.h>

#include "test_traits_adaptor.h"
#include "Traits_adaptor_test.h"

int main (int argc, char * argv[])
{
  Traits_adaptor_test<Traits> test;
  if (!test.parse(argc, argv)) return -1;
  if (!test.init()) return -1;
  if (!test.perform()) return -1;
  return 0;
}