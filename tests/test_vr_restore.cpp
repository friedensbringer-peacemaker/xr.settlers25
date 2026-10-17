#include "driver/VrInput.h"
#include <cassert>
#include <initializer_list>
int main() {
 for(bool ix:{false,true}) for(bool iy:{false,true}) for(bool sw:{false,true}) {
  auto a=vrInput::mapAxes(.3f,-.7f,ix,iy,sw);
  const float x=ix ? -.3f:.3f, y=iy ? .7f:-.7f;
  assert(a.x==(sw?y:x) && a.y==(sw?x:y));
 }
 vrInput::ZoomRepeat z;
 assert(z.sample(1,false,0)==0);assert(z.sample(.3f,true,1)==0);
 assert(z.sample(1,true,10)==1);assert(z.sample(1,true,459)==0);
 assert(z.sample(1,true,460)==1);assert(z.sample(1,true,709)==0);
 assert(z.sample(-1,true,710)==-1);assert(z.sample(-1,false,711)==0);
 assert(z.sample(-1,true,712)==-1);assert(z.sample(0,true,713)==0);
}
