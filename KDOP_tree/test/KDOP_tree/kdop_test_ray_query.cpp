/*
 * kdop_test_aabb_kdop_tree.cpp
 *
 *  Created on: 20 Jun 2019
 *      Author: xx791
 */

//#define CHECK_CORRECTNESS

#define AABB_TIMING
#define KDOP_TIMING

//#define WRITE_FILE

#include <iostream>
#include <fstream>
#include <list>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>

// AABB tree includes
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
//#define DEBUG_
// KDOP tree includes
#include <CGAL/KDOP_tree/KDOP_tree.h>
#include <CGAL/KDOP_tree/KDOP_traits.h>

#include <CGAL/Timer.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::FT FT;
typedef K::Point_3 Point;
typedef K::Vector_3 Vector;
typedef K::Ray_3 Ray;
typedef K::Segment_3 Segment;

typedef CGAL::Surface_mesh<Point> Mesh;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;

// AABB tree type definitions
typedef CGAL::AABB_face_graph_triangle_primitive<Mesh> Primitive_aabb;
typedef CGAL::AABB_traits<K, Primitive_aabb> Traits_aabb;
typedef CGAL::AABB_tree<Traits_aabb> Tree_aabb;

typedef boost::optional< Tree_aabb::Intersection_and_primitive_id<Ray>::Type > Ray_intersection;

// KDOP tree type definitions
const unsigned int NUM_DIRECTIONS = 14;

typedef CGAL::AABB_face_graph_triangle_primitive<Mesh> Primitive_kdop;
typedef CGAL::KDOP_tree::KDOP_traits<NUM_DIRECTIONS, K, Primitive_kdop> Traits_kdop;
typedef CGAL::KDOP_tree::KDOP_tree<Traits_kdop> Tree_kdop;

typedef CGAL::Timer Timer;

void read_points(std::ifstream& pointsf, std::vector<Point>& points);

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Need mesh file and points file!" << std::endl;
    return 0;
  }

  const char* filename = argv[1];
  std::ifstream input(filename);

  Mesh mesh;
  input >> mesh;

  const char* pointsFile = argv[2];
  std::ifstream pointsf(pointsFile);

  std::cout << "read points from file" << std::endl;
  std::vector<Point> points;
  read_points(pointsf, points);

  // create a set of random rays, centred at points read from the file.
  std::vector< Ray > rays;

  std::cout << "create rays from points" << std::endl;

  double d = CGAL::Polygon_mesh_processing::is_outward_oriented(mesh)?-1:1;

  for(face_descriptor fd : faces(mesh)){
    halfedge_descriptor hd = halfedge(fd,mesh);
    Point p = CGAL::centroid(mesh.point(source(hd,mesh)),
        mesh.point(target(hd,mesh)),
        mesh.point(target(next(hd,mesh),mesh)));

/*
    Vector v = CGAL::Polygon_mesh_processing::compute_face_normal(fd,mesh);
    Ray ray(p, d*v);

    //Ray ray(points[0], p);
    
    rays.push_back(ray);
*/

    for (int i = 0; i < points.size(); ++i) {
      Ray ray(points[i], p);
      rays.push_back(ray);
    }

  }

#ifdef WRITE_FILE

  // write rays to file
  std::string rayFile("ray_file_test.obj");
  std::ofstream rayf(rayFile.c_str());

  for (int i = 0; i < rays.size(); ++i) {
    Ray ray = rays[i];

    Point source = ray.source();
    Point target = ray.second_point();

    rayf << "v " << source.x() << " " << source.y() << " " << source.z() << std::endl;
    rayf << "v " << target.x() << " " << target.y() << " " << target.z() << std::endl;
  }

  for (int i = 0; i < rays.size(); ++i) {
    rayf << "l " << 2*i + 1 << " " << 2*i + 2 << std::endl;
  }
#endif

  Timer t;

#ifdef AABB_TIMING  
  //===========================================================================
  // AABB tree build
  //===========================================================================
  t.start();
  Tree_aabb tree_aabb( faces(mesh).first, faces(mesh).second, mesh );

  tree_aabb.build();
  t.stop();
  std::cout << "Build time AABB tree: " << t.time() << " sec."<< std::endl;
#endif
  
  //===========================================================================
  // KDOP tree build
  //===========================================================================

#ifdef KDOP_TIMING
  t.reset();
  t.start();
  Tree_kdop tree_kdop( faces(mesh).first, faces(mesh).second, mesh );

  // build the tree, including splitting primitives and computing k-dops
  tree_kdop.build();
  t.stop();
  std::cout << "Build time " << NUM_DIRECTIONS << "-DOP tree: " << t.time() << " sec."<< std::endl << std::endl;
#endif
  
  //===========================================================================
  // Ray intersection check using AABB tree and KDOP tree
  //===========================================================================

#ifdef CHECK_CORRECTNESS
  
  int num_error = 0;
  for (int i = 0; i < rays.size(); ++i) {
    std::cout << "ray " << i << "\r";
    const Ray& ray_query = rays[i];
    
    // AABB tree
    bool is_intersect_aabb = tree_aabb.do_intersect(ray_query);
    
    // KDOP tree
    bool is_intersect_kdop = tree_kdop.do_intersect(ray_query);
    
    if (is_intersect_aabb != is_intersect_kdop) {
      std::cout << "ERROR!" << std::endl;
      num_error += 1;
    }

    /*
    Ray_intersection intersection_aabb = tree_aabb.first_intersection(ray_query);
    Ray_intersection intersection_kdop = tree_kdop.first_intersection(ray_query);

    const Point* p_aabb = boost::get<Point>( &(intersection_aabb->first) );
    const Point* p_kdop = boost::get<Point>( &(intersection_kdop->first) );

    bool is_same = K().equal_3_object()(*p_aabb, *p_kdop);

    if (is_same == false) {
      std::cout << "ERROR: first_intersection!" << std::endl;
      num_error += 1;
    }
    */
  }
  
  if (num_error == 0){
    std::cout << "The do_intersect result of KDOP is the same as AABB." << std::endl;
  } else {
    std::cout << num_error << " differences for " << rays.size() << " queries" << std::endl;
    return -1;
  }
#endif

#ifdef AABB_TIMING
  t.reset();
  t.start();
  for (int i = 0; i < rays.size(); ++i) {
    const Ray& ray_query = rays[i]; 
    bool is_intersect = tree_aabb.do_intersect(ray_query);
    //Ray_intersection intersection = tree_aabb.first_intersection(ray_query);
  }
  t.stop();
  std::cout << t.time() << " sec. for "   << rays.size() << " queries with an AABB tree" << std::endl;
#endif

#ifdef KDOP_TIMING
  t.reset();
  t.start();
  for (int i = 0; i < rays.size(); ++i) {
    const Ray& ray_query = rays[i];
    bool is_intersect = tree_kdop.do_intersect(ray_query);
    //Ray_intersection intersection = tree_kdop.first_intersection(ray_query);
  }
  t.stop();
  std::cout << t.time() << " sec. for "  << rays.size() << " queries with a " << NUM_DIRECTIONS << "-DOP tree" << std::endl;
#endif
  
  return 0;
}

void read_points(std::ifstream& pointsf, std::vector<Point>& points)
{
  std::string line;
  while ( std::getline(pointsf, line) ) {
    std::stringstream line_stream;
    line_stream.str(line);
    double x, y, z;
    line_stream >> x >> y >> z;

    Point p(x, y, z);

    points.push_back(p);
  }
}
