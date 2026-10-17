#include "HqAsset.h"
#include <cassert>
#include <iostream>
int main(int argc,char**argv){
 // Orthographic map coordinates, including scrolling/zoom, must keep the HQ anchored.
 float model[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
 const float projection[16]={.002f,0,0,0,0,-.004f,0,0,0,0,1,0,-1,1,0,1};
 auto p=vrHq::project(500,250,model,projection);
 assert(std::fabs(p[0]-.5f)<1e-5f && std::fabs(p[1]-.5f)<1e-5f);
 model[0]=2;model[5]=2;model[12]=-500;model[13]=-250;
 auto q=vrHq::project(500,250,model,projection);
 assert(std::fabs(q[0]-p[0])<1e-5f && std::fabs(q[1]-p[1])<1e-5f);
 auto right=vrHq::project(565,250,model,projection);
 assert(std::fabs(right[0]-q[0]-.13f)<1e-5f);
 vrHq::Asset a;assert(!a.load("/tmp/no-such-asset.xrhq"));
 {std::ofstream f("/tmp/truncated-hq.xrhq",std::ios::binary);std::array<uint32_t,6> h{0x51485258,1,3,3,1,1};f.write(reinterpret_cast<char*>(h.data()),sizeof(h));}
 assert(!a.load("/tmp/truncated-hq.xrhq"));
 if(argc>1){assert(a.load(argv[1]));assert(a.indices.size()%3==0);std::cout<<a.indices.size()/3<<" triangles validated\n";}
}
