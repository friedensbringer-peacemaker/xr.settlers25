// SPDX-License-Identifier: GPL-2.0-or-later
#include "PanelGeometry.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace vrPanel;
bool near(float a,float b) { return std::fabs(a-b)<0.0002f; }
int main() {
 for(float aspect : {1.6f, 1920.f/1080.f, 2560.f/1080.f, 3440.f/1440.f, 3840.f/1080.f})
 for(int shape=0; shape<=2; ++shape) {
  const float arc=angle(shape), width=2.f,height=width/aspect,z=-1.8f;
  for(float u : {0.01f,0.25f,0.5f,0.75f,0.99f}) for(float v : {0.01f,0.5f,0.99f}) {
   const float theta=(u-.5f)*arc;
   const Vec3 point = arc==0 ? Vec3{(u-.5f)*width,(.5f-v)*height,z}
     : Vec3{width/arc*std::sin(theta),(.5f-v)*height,z+width/arc*(1-std::cos(theta))};
   for(Vec3 origin : {Vec3{0,0,0},Vec3{.35f,-.2f,-.3f}}) {
    const Vec3 dir{point.x-origin.x,point.y-origin.y,point.z-origin.z};
    Hit hit;
    assert(intersect(origin,dir,width,height,z,arc,hit));
    assert(near(hit.u,u)&&near(hit.v,v));
    assert(near(hit.point.x,point.x)&&near(hit.point.z,point.z));
   }
  }
  Hit h;
  assert(!intersect({0,0,0},{0,0,1},width,height,z,arc,h));
  assert(!intersect({0,0,0},{0,1,0},width,height,z,arc,h));
  assert(!intersect({0,0,0},{0,4,-1.8f},width,height,z,arc,h));
  assert(!intersect({0,0,0},{10,0,-1.8f},width,height,z,arc,h));
 }
 std::cout << "450 flat/curved UV intersections and 60 miss cases across 5 aspect ratios passed\n";
}
