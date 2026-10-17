// SPDX-License-Identifier: GPL-2.0-or-later
#include "driver/VrInput.h"
#include "driver/VrSettings.h"
#include "SaveName.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
int main()
{
    vrInput::StickTurbo t;
    assert(!t.sample(0,1,100)); assert(!t.sample(0,0,160));
    assert(t.sample(0,1,300));assert(t.sample(0,1,1200));
    assert(!t.sample(0,0,1250));assert(!t.sample(1,0,1300));
    assert(!t.sample(0,0,1400));assert(!t.sample(1,0,1800));
    assert(!t.sample(-1,0,1850));assert(!t.sample(0,0,1900));assert(t.sample(-1,0,2100));
    t={};assert(!t.sample(-1,0,2200)); // focus change resets boost
    int cases=0;
    for(int fps:{36,72,90,120}) for(int speed:{0,1,2}) for(bool turbo:{false,true}) {
        vrInput::PanAccumulator p;
        int sum=0;
        for(int i=0;i<fps;++i) { int x,y;p.sample(1,0,1.f/fps,speed,turbo,x,y);sum+=x;assert(y==0); }
        assert(std::abs(sum-200*(1<<speed)*(turbo?3:1))<=1);
        p={};int sx=0,sy=0;
        for(int i=0;i<fps;++i) { int x,y;p.sample(1,1,1.f/fps,speed,turbo,x,y);sx+=x;sy+=y; }
        assert(std::fabs(std::sqrt(float(sx*sx+sy*sy))-200*(1<<speed)*(turbo?3:1))<2);
        int x,y;p.sample(.2f,-.2f,.01f,speed,turbo,x,y);assert(x==0&&y==0);
        ++cases;
    }
    VrSettings s;
    assert(s.panelControl==2 && s.mapSpeed==1);
    s.screenDistanceMm=2300;s.tableDistanceMm=900;
    assert(std::fabs(s.restoredDistance()-2.3f)<.001f);
    s.presentation=1;assert(std::fabs(s.restoredDistance()-.9f)<.001f);
    s.rememberDistance=false;assert(s.restoredDistance()==s.viewStartDistance());
    s.rememberDistance=true;s.tableDistanceMm=0;assert(s.restoredDistance()==s.viewStartDistance());
    assert(saveName::stem("2026-09-26_19-30-00","Spieler","Europa")=="2026-09-26_19-30-00-Spieler-Europa");
    assert(saveName::component("../A:B*?\\x","x")=="A-B---x");
    assert(saveName::component(" .- ","Karte")=="Karte");
    assert(saveName::component(std::string(63,'a')+"ä", "x")==std::string(63,'a'));
    assert(saveName::component("Südsee","x")=="Südsee");
    // Trigger map grab: tap stays a click, movement grabs, 6 cm towards/away zooms one step each.
    vrInput::MapGrab g;
    g.begin(100,100,0.f);assert(g.engaged()&&g.sample(105,104,.01f)==0&&g.pending);assert(g.end()&&!g.engaged());
    g.begin(100,100,0.f);assert(g.sample(120,100,0.f)==0&&g.active);assert(!g.end());
    g.begin(100,100,0.f);assert(g.sample(100,100,.04f)==0&&g.active);
    assert(g.sample(100,100,.059f)==0&&g.sample(100,100,.061f)==1&&g.sample(100,100,.1f)==0&&g.sample(100,100,.121f)==1);
    assert(g.sample(100,100,.0f)==-1&&g.sample(100,100,-.01f)==-1&&g.sample(100,100,-.01f)==0);assert(!g.end());
    std::cout<<cases<<" frame-rate/speed/diagonal cases; boost gestures, map grab, restored distances and filename sanitization passed\n";
}
