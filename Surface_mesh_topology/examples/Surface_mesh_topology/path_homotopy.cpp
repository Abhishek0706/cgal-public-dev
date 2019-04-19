#include <CGAL/Linear_cell_complex_for_combinatorial_map.h>
#include <CGAL/Linear_cell_complex_constructors.h>
#include <CGAL/Surface_mesh_curve_topology.h>

/* If you want to use a viewer, you can use qglviewer. */
#ifdef CGAL_USE_BASIC_VIEWER
#include <CGAL/draw_lcc_with_paths.h>
#endif

#include <CGAL/Path_generators.h>
#include <CGAL/Path_on_surface.h>

typedef CGAL::Linear_cell_complex_for_combinatorial_map<2,3> LCC_3_cmap;
///////////////////////////////////////////////////////////////////////////////
[[ noreturn ]] void usage(int /*argc*/, char** argv)
{
  std::cout<<"usage: "<<argv[0]<<" file [-draw] [-L l1 l2] [-D d1 d2] "
           <<" [-N n] [-seed S] [-time]"<<std::endl
           <<"   Load the given off file, compute one random path, deform it "
           <<"into a second path and test that the two paths are isotopic."
           <<std::endl
           <<"   -draw: draw mesh and the two paths." <<std::endl
           <<"   -L l1 l2: create a path of length >= l: a random number between [l1, l2] (by default [10, 100])."<<std::endl
           <<"   -D d1 d2: use d deformations to generate the second path: d is a random number between [d1, d2] (by default [10, 100]."<<std::endl
           <<"   -N n: do n tests of isotopy (using 2*n random paths) (by default 1)."<<std::endl
           <<"   -seed S: uses S as seed of random generator. Otherwise use a different seed at each run (based on time)."<<std::endl
           <<"   -time: display computation times."<<std::endl
           <<std::endl;
  exit(EXIT_FAILURE);
}
///////////////////////////////////////////////////////////////////////////////
[[ noreturn ]] void error_command_line(int argc, char** argv, const char* msg)
{
  std::cout<<"ERROR: "<<msg<<std::endl;
  usage(argc, argv);
}
///////////////////////////////////////////////////////////////////////////////
void process_command_line(int argc, char** argv,
                          std::string& file,
                          bool& draw,
                          unsigned int& l1,
                          unsigned int& l2,
                          unsigned int& d1,
                          unsigned int& d2,
                          unsigned int& N,
                          CGAL::Random& random,
                          bool& time)
{
  std::string arg;
  for (int i=1; i<argc; ++i)
  {
    arg=argv[i];
    if (arg=="-draw")
    { draw=true; }
    else if (arg=="-D")
    {
     if (i+2>=argc)
     { error_command_line(argc, argv, "Error: not enough number after -D option."); }
     d1=static_cast<unsigned int>(std::stoi(std::string(argv[++i])));
     d2=static_cast<unsigned int>(std::stoi(std::string(argv[++i])));
    }
    else if (arg=="-L")
    {
     if (i+2>=argc)
     { error_command_line(argc, argv, "Error: not enough number after -L option."); }
     l1=static_cast<unsigned int>(std::stoi(std::string(argv[++i])));
     l2=static_cast<unsigned int>(std::stoi(std::string(argv[++i])));
    }
    else if (arg=="-N")
    {
      if (i==argc-1)
      { error_command_line(argc, argv, "Error: no number after -nbtests option."); }
      N=static_cast<unsigned int>(std::stoi(std::string(argv[++i])));
    }
    else if (arg=="-seed")
    {
      if (i==argc-1)
      { error_command_line(argc, argv, "Error: no number after -seed option."); }
      random=CGAL::Random(static_cast<unsigned int>(std::stoi(std::string(argv[++i])))); // initialize the random generator with the given seed
    }
    else if (arg=="-time")
    { time=true; }
    else if (arg=="-h" || arg=="--help" || arg=="-?")
    { usage(argc, argv); }
    else if (arg[0]=='-')
    { std::cout<<"Unknown option "<<arg<<", ignored."<<std::endl; }
    else
    { file=arg; }
  }

  if (N==0) { N=1; }
  if (l2<l1) l2=l1;
  if (d2<d1) d2=d1;
}
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  std::string file="data/3torus-smooth.off";  
  bool draw=false;
  unsigned int l1=10, l2=100;
  unsigned int d1=10, d2=100;
  unsigned int N=1;
  CGAL::Random random; // Used when user do not provide its own seed.
  bool time=false;

  process_command_line(argc, argv, file, draw, l1, l2, d1, d2, N, random, time);

  LCC_3_cmap lcc;
  if (!CGAL::load_off(lcc, file.c_str()))
  {
    std::cout<<"PROBLEM reading file "<<file<<std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout<<"Initial map: ";
  lcc.display_characteristics(std::cout) << ", valid="
                                         << lcc.is_valid() << std::endl;
  
  CGAL::Surface_mesh_curve_topology<LCC_3_cmap> smct(lcc, time);
  std::cout<<"Reduced map: ";
  smct.get_map().display_characteristics(std::cout)
    << ", valid="<< smct.get_map().is_valid() << std::endl;
   
  unsigned int nbcontractible=0;
  std::vector<std::size_t> errors_seeds;
  unsigned int length, defo;
  
  for (unsigned int i=0; i<N; ++i)
  {
    // TEMPO POUR DEBUG
    //random=CGAL::Random(461974893);
    // END TEMPO POUR DEBUG

    if (i!=0)
    {
      random=CGAL::Random(random.get_int(0, std::numeric_limits<int>::max()));
    }
    std::cout<<"Random seed: "<<random.get_seed()<<std::endl;

    length=static_cast<unsigned int>(random.get_int(l1, l2+1));
    defo=static_cast<unsigned int>(random.get_int(d1, d2+1));
    
    std::vector<CGAL::Path_on_surface<LCC_3_cmap> > paths;
    std::vector<CGAL::Path_on_surface<LCC_3_cmap> > transformed_paths;

    CGAL::Path_on_surface<LCC_3_cmap> path1(lcc);
    path1.generate_random_closed_path(length, random);

    //if (path1.length()<100000) { // TEMPO FOR DEBUG
    std::cout<<"Path1 size: "<<path1.length()<<" (from "<<length<<" darts); ";
    paths.push_back(path1);

    CGAL::Path_on_surface<LCC_3_cmap> path2(path1);
    path2.update_path_randomly(defo, random);
    std::cout<<"Path2 size: "<<path2.length()<<" (from "<<defo<<" deformations): ";
    paths.push_back(path2);
    std::cout<<std::flush;

    if (smct.is_contractible(path1, time))
    { ++nbcontractible; }

    bool res=smct.are_freely_homotopic(path1, path2, time);
    if (!res)
    {
      /* std::cout<<"ERROR: paths are not homotopic while they should be."
               <<std::endl; */
      errors_seeds.push_back(random.get_seed());
    }
    /* else
    { std::cout<<"TEST OK: paths are homotopic."<<std::endl; } */

#ifdef CGAL_USE_BASIC_VIEWER
    if (draw)
    { CGAL::draw(lcc, paths); }
#endif
     // END TEMPO POUR DEBUG}

   //  { if (!res) { i=0; errors_seeds.clear();} } // TEMPO POUR DEBUG
  }

  if (errors_seeds.empty())
  {
    if (N==1) { std::cout<<"Test OK: both paths are homotopic."<<std::endl; }
    else { std::cout<<"All the "<<N<<" tests OK: each pair of paths were homotopic."<<std::endl; }
  }
  else
  {
    std::cout<<"ERRORS for "<<errors_seeds.size()<<" tests among "<<N
            <<" (i.e. "<<(double)(errors_seeds.size()*100)/double(N)<<"%)."<<std::endl;
    std::cout<<"Errors for seeds: ";
    for (std::size_t i=0; i<errors_seeds.size(); ++i)
    { std::cout<<errors_seeds[i]<<"  "; }
    std::cout<<std::endl;
  }

  std::cout<<"Number of contractible paths: "<<nbcontractible<<" among "<<N
           <<" (i.e. "<<(double)(nbcontractible*100)/double(N)<<"%)."<<std::endl;

  return EXIT_SUCCESS;
}