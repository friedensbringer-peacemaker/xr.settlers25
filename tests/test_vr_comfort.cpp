// SPDX-License-Identifier: GPL-2.0-or-later
#include "PanelGeometry.h"
#include "driver/VrInput.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using namespace vrPanel;
bool close(float a,float b) { return std::fabs(a-b)<0.0003f; }
int main()
{
    const auto quarter=rotateYaw({0,0,-2},1.570796327f);
    assert(close(quarter.x,-2) && close(quarter.z,0));
    unsigned checks=0;
    for(float yaw : {-3.f,-1.57f,-0.3f,0.f,0.8f,1.57f,3.f})
    for(Vec3 origin : {Vec3{0,0,0},Vec3{1.3f,1.7f,-0.8f}})
    for(int shape=0;shape<3;++shape)
    for(float u : {0.02f,0.5f,0.98f})
    for(float v : {0.02f,0.5f,0.98f})
    {
        const float arc=angle(shape), theta=(u-.5f)*arc;
        const Vec3 localTarget=arc==0 ? Vec3{(u-.5f)*2,(.5f-v)*1.25f,-1.8f}
          : Vec3{2/arc*std::sin(theta),(.5f-v)*1.25f,-1.8f+2/arc*(1-std::cos(theta))};
        const auto worldTarget=worldPoint(localTarget,origin,yaw);
        const auto worldRayOrigin=worldPoint({0.12f,-0.15f,0},origin,yaw);
        const auto localOrigin=localPoint(worldRayOrigin,origin,yaw);
        const auto localDirection=rotateYaw({worldTarget.x-worldRayOrigin.x,worldTarget.y-worldRayOrigin.y,worldTarget.z-worldRayOrigin.z},-yaw);
        Hit hit;
        assert(intersect(localOrigin,localDirection,2,1.25f,-1.8f,arc,hit));
        assert(close(hit.u,u) && close(hit.v,v));
        ++checks;
    }
    assert(vrInput::confirmationSeconds(100,100)==15);
    assert(vrInput::confirmationSeconds(100,15099)==1);
    assert(vrInput::confirmationSeconds(100,15100)==0);
    assert(vrInput::confirmationSeconds(100,17000)==0);
    assert(vrInput::confirmationSeconds(std::numeric_limits<unsigned>::max()-499,500)==14);
    vrInput::ButtonGesture button;
    assert(button.sample(true,true,100,3,0)==3);
    assert(button.sample(true,true,1000,3,0)==0);
    assert(button.sample(false,true,1100,3,0)==0);
    assert(button.sample(true,true,2000,3,7)==0);
    assert(button.sample(false,true,2300,3,7)==3);
    assert(button.sample(true,true,3000,3,7)==0);
    assert(button.sample(true,true,3599,3,7)==0);
    assert(button.sample(true,true,3600,3,7)==7);
    assert(button.sample(true,true,4200,3,7)==0);
    assert(button.sample(false,true,4500,3,7)==0);
    assert(button.sample(true,true,5000,3,7)==0);
    assert(button.sample(false,false,5300,3,7)==0); // tracking/focus loss must not fire short press
    assert(button.sample(false,true,5500,3,7)==0);
    assert(button.sample(true,true,6000,0,7)==0);
    assert(button.sample(false,true,6200,0,7)==0);
    for(int strength : {1,2}) {
        float one=vrInput::smoothingAlpha(strength,.02f);
        float half=vrInput::smoothingAlpha(strength,.01f);
        assert(close(one,half+(1-half)*half)); // independent of frame rate
        assert(one>0 && one<1);
    }
    assert(vrInput::smoothingAlpha(0,.01f)==1);
    std::cout << checks << " rotated flat/cylinder ray hits; short/long/disabled/lost-input gestures and smoothing passed\n";
}
