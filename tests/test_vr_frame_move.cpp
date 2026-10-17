#include "PanelGeometry.h"
#include <cassert>
#include <cmath>
int main() {
 using namespace vrPanel;
 assert(moveFrameHit(.005f,.5f));assert(moveFrameHit(.995f,.5f));
 assert(moveFrameHit(.5f,.01f));assert(moveFrameHit(.2f,.99f));
 assert(!moveFrameHit(.5f,.99f));assert(!moveFrameHit(.5f,.5f));
 assert(!moveFrameHit(-.1f,.5f));assert(!moveFrameHit(.5f,1.01f));
 unsigned cases=0;
 for(float yaw:{-.7f,0.f,.9f}) for(float pitch:{-1.0f,0.f,.3f}) {
  Pose start{{.2f,.1f,.3f},yaw,-1.8f,pitch,-.4f};
  for(Vec3 anchor: {Vec3{-.9f,0,-1.8f},Vec3{.9f,.2f,-1.6f},Vec3{0,.6f,-1.8f}}) {
   const auto world=start.toWorld(anchor);const Vec3 hand{.3f,-.2f,.1f};
   TranslationDrag drag;drag.begin(start.origin,world,hand);
   const Vec3 dir{(world.x-hand.x)/drag.distance,(world.y-hand.y)/drag.distance,(world.z-hand.z)/drag.distance};
   for(Vec3 delta: {Vec3{0,0,0},Vec3{.4f,0,0},Vec3{0,-.3f,0},Vec3{0,0,.5f}}) {
    Pose moved=start;moved.origin=drag.translated({hand.x+delta.x,hand.y+delta.y,hand.z+delta.z},dir);
    const auto end=moved.toWorld(anchor);
    assert(std::fabs(end.x-world.x-delta.x)<1e-5f);
    assert(std::fabs(end.y-world.y-delta.y)<1e-5f);
    assert(std::fabs(end.z-world.z-delta.z)<1e-5f);
    assert(moved.yaw==start.yaw && moved.pitch==start.pitch && moved.centerZ==start.centerZ && moved.height==start.height);
    ++cases;
   }
   const Vec3 turned{-dir.x,dir.y,dir.z};Pose moved=start;moved.origin=drag.translated(hand,turned);
   const auto end=moved.toWorld(anchor),target=drag.target(hand,turned);
   assert(std::fabs(end.x-target.x)<1e-5f && std::fabs(end.y-target.y)<1e-5f && std::fabs(end.z-target.z)<1e-5f);
   assert(moved.yaw==yaw && moved.pitch==pitch);
  }
 }
 assert(cases==108);
}
