// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <vector>
#include <fstream>
#include <cmath>
#include <cstdint>
namespace vrHq {
struct Asset {
    std::vector<float> vertices; // xyz uv, already in board coordinates
    std::vector<uint32_t> indices;
    std::vector<unsigned char> pixels;
    uint32_t width=0,height=0;
    bool load(const char* path) {
        std::ifstream f(path,std::ios::binary);
        std::array<uint32_t,6> h{};
        if(!f.read(reinterpret_cast<char*>(h.data()),sizeof(h)) || h[0]!=0x51485258 || h[1]!=1
           || h[2]==0 || h[2]>300000 || h[3]==0 || h[3]>900000 || h[3]%3 || h[4]==0 || h[5]==0 || h[4]>2048 || h[5]>2048) return false;
        Asset a;a.width=h[4];a.height=h[5];a.vertices.resize(size_t(h[2])*5);a.indices.resize(h[3]);a.pixels.resize(size_t(h[4])*h[5]*4);
        if(!f.read(reinterpret_cast<char*>(a.vertices.data()),a.vertices.size()*sizeof(float))
          || !f.read(reinterpret_cast<char*>(a.indices.data()),a.indices.size()*sizeof(uint32_t))
          || !f.read(reinterpret_cast<char*>(a.pixels.data()),a.pixels.size())) return false;
        for(float v:a.vertices)if(!std::isfinite(v) || std::fabs(v)>16)return false;
        for(auto i:a.indices)if(i>=h[2])return false;
        *this=std::move(a);return true;
    }
};
// Apply engine zoom and scrolling when locating the building on the tabletop.
inline std::array<float,2> project(float x,float y,const float* model,const float* projection)
{
    float eye[4],clip[4];
    for(int i=0;i<4;++i) eye[i]=model[i]*x+model[4+i]*y+model[12+i];
    for(int i=0;i<4;++i) {clip[i]=0;for(int j=0;j<4;++j)clip[i]+=projection[j*4+i]*eye[j];}
    if(std::fabs(clip[3])<1e-6f) return {{-10,-10}};
    return {{.5f+.5f*clip[0]/clip[3],.5f-.5f*clip[1]/clip[3]}};
}
}
