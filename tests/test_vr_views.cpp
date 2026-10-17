// SPDX-License-Identifier: GPL-2.0-or-later
#include "PanelGeometry.h"
#include "driver/VrSettings.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace vrPanel;
bool near(float a,float b) { return std::fabs(a-b)<0.0003f; }
void same(Vec3 a,Vec3 b) { assert(near(a.x,b.x)&&near(a.y,b.y)&&near(a.z,b.z)); }
Vec3 rotateQuaternion(Vec3 v,Quaternion q)
{
    const Vec3 t{2*(q.y*v.z-q.z*v.y),2*(q.z*v.x-q.x*v.z),2*(q.x*v.y-q.y*v.x)};
    return {v.x+q.w*t.x+q.y*t.z-q.z*t.y,
            v.y+q.w*t.y+q.z*t.x-q.x*t.z,
            v.z+q.w*t.z+q.x*t.y-q.y*t.x};
}
int main()
{
    VrSettings settings;
    assert(settings.presentation==0 && settings.viewPitch()==0 && settings.viewHeight()==0);
    assert(settings.viewCurvature()==1);
    settings.presentation=1;
    assert(settings.viewCurvature()==0 && near(settings.viewHeight(),-.4f));
    assert(near(settings.viewStartDistance(),1.2f));
    // Horizontal tabletop: top edge must be further away; its front faces upward.
    Pose horizontal{{0,1.6f,0},0,-1.2f,-1.570796327f,-.65f};
    same(horizontal.toWorld({0,0,-1.2f}),{0,.95f,-1.2f});
    same(horizontal.toWorld({0,.5f,-1.2f}),{0,.95f,-1.7f});
    same(rotateQuaternion({0,0,1},horizontal.orientation()),{0,1,0});
    unsigned hits=0;
    for(int mode : {0,1}) for(int tilt=0;tilt<4;++tilt) for(int height=0;height<10;++height)
    for(int distance=0;distance<3;++distance) for(float yaw : {-2.3f,0.f,1.4f})
    for(float width : {1.f,2.f,3.2f}) for(float aspect : {1.6f,3.55555556f})
    {
        settings.presentation=mode;settings.tableTilt=tilt;settings.tableLoweringCm=height*10;
        settings.tableDistance=distance;settings.startDistance=distance;
        const float z=-settings.viewStartDistance(), arc=angle(settings.viewCurvature());
        const Vec3 origin{1.2f,1.65f,-.3f};
        const Pose pose{origin,yaw,z,settings.viewPitch(),settings.viewHeight()};
        // Tilting preserves the center and places it at the chosen height.
        same(pose.toWorld({0,0,z}),worldPoint({0,settings.viewHeight(),z},origin,yaw));
        for(Vec3 axis : {Vec3{1,0,0},Vec3{0,1,0},Vec3{0,0,1}})
            same(pose.directionToLocal(rotateQuaternion(axis,pose.orientation())),axis);
        for(float u : {.01f,.5f,.99f}) for(float v : {.01f,.5f,.99f})
        {
            const float theta=(u-.5f)*arc;
            const Vec3 target=arc==0 ? Vec3{(u-.5f)*width,(.5f-v)*width/aspect,z}
              : Vec3{width/arc*std::sin(theta),(.5f-v)*width/aspect,z+width/arc*(1-std::cos(theta))};
            const auto worldTarget=pose.toWorld(target);
            same(pose.toLocal(worldTarget),target);
            if(mode==0) same(worldTarget,worldPoint(target,origin,yaw)); // old screen unchanged
            // Controller remains by the viewer while the board tilts below it.
            const auto rayOrigin=worldPoint({.2f,-.2f,0},origin,yaw);
            const auto dir=pose.directionToLocal({worldTarget.x-rayOrigin.x,worldTarget.y-rayOrigin.y,worldTarget.z-rayOrigin.z});
            Hit hit;
            assert(intersect(pose.toLocal(rayOrigin),dir,width,width/aspect,z,arc,hit));
            assert(near(hit.u,u)&&near(hit.v,v));
            // A ray from behind must not click the mirrored back side.
            const auto backOrigin=pose.toWorld({target.x,target.y,target.z-.1f});
            const auto backDir=pose.directionToLocal({worldTarget.x-backOrigin.x,worldTarget.y-backOrigin.y,worldTarget.z-backOrigin.z});
            assert(!intersect(pose.toLocal(backOrigin),backDir,width,width/aspect,z,arc,hit));
            ++hits;
        }
    }
    std::cout<<hits<<" view ray hits and back-face misses; center, orientation and classic compatibility passed\n";
}
