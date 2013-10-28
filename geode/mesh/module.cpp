//#####################################################################
// Module mesh
//#####################################################################
#include <geode/python/wrap.h>
using namespace geode;

void wrap_mesh() {
  GEODE_WRAP(polygon_mesh)
  GEODE_WRAP(segment_soup)
  GEODE_WRAP(triangle_mesh)
  GEODE_WRAP(triangle_subdivision)
  GEODE_WRAP(halfedge_mesh)
  GEODE_WRAP(corner_mesh)
}