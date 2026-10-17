# SPDX-License-Identifier: GPL-2.0-or-later
# Dependencies: numpy, Pillow, meshoptimizer==0.2.30a0 (isolated venv recommended).
import numpy as np,meshoptimizer as mo,struct,json,io,ctypes
from meshoptimizer.simplifier import lib
lib.meshopt_simplifyWithAttributes.argtypes=[ctypes.POINTER(ctypes.c_uint),ctypes.POINTER(ctypes.c_uint),ctypes.c_size_t,ctypes.POINTER(ctypes.c_float),ctypes.c_size_t,ctypes.c_size_t,ctypes.POINTER(ctypes.c_float),ctypes.c_size_t,ctypes.POINTER(ctypes.c_float),ctypes.c_size_t,ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,ctypes.c_float,ctypes.c_uint,ctypes.POINTER(ctypes.c_float)]
lib.meshopt_simplifyWithAttributes.restype=ctypes.c_size_t
from pathlib import Path
from PIL import Image
import argparse
parser=argparse.ArgumentParser(description='Convert an identity-transform, single-mesh shaded GLB to a Quest HQ asset')
parser.add_argument('source',type=Path);parser.add_argument('output',type=Path)
args=parser.parse_args();src=args.source;out=args.output;out.mkdir(parents=True,exist_ok=True)
b=src.read_bytes();n=struct.unpack_from('<I',b,12)[0];j=json.loads(b[20:20+n]);binary=b[28+n:]
assert len(j['meshes'])==1 and len(j['meshes'][0]['primitives'])==1
assert all(not any(k in node for k in ['matrix','translation','rotation','scale']) for node in j['nodes'])
assert j['materials'][0].get('emissiveTexture',{}).get('index')==0, 'Use the shaded export with an emissive texture'
assert all(j['meshes'][0]['primitives'][0]['attributes'][k]==v for k,v in [('POSITION',0),('TEXCOORD_0',2)])
def view(i):
 v=j['bufferViews'][i];return binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']]
def acc(i):
 a=j['accessors'][i];v=j['bufferViews'][a['bufferView']];assert 'byteStride' not in v
 return np.frombuffer(view(a['bufferView']),dtype={5126:'<f4',5125:'<u4',5123:'<u2'}[a['componentType']],offset=a.get('byteOffset',0),count=a['count']*{'VEC3':3,'VEC2':2,'SCALAR':1}[a['type']]).reshape(a['count'],-1).copy()
p=acc(0);uv=acc(2);idx=acc(3).ravel().astype(np.uint32)
source_triangles=len(idx)//3
print('Source',source_triangles,'triangles',p.min(0),p.max(0),flush=True)
joined,remap=np.unique(np.column_stack((p,uv)),axis=0,return_inverse=True)
p=np.ascontiguousarray(joined[:,:3]);uv=np.ascontiguousarray(joined[:,3:]);idx=remap[idx].astype(np.uint32)
print('Welded vertices',len(p),flush=True)
dst=np.empty_like(idx);count=mo.simplify_with_attributes(dst,idx,p,uv,np.array([.1,.1],np.float32),target_index_count=150000,target_error=.025)
idx=dst[:count];used,idx=np.unique(idx,return_inverse=True);p=p[used];uv=uv[used]
lo=p.min(0);hi=p.max(0);scale=1.28/max(hi[0]-lo[0],hi[2]-lo[2]);v=np.column_stack(((p[:,0]-(lo[0]+hi[0])/2)*scale,-(p[:,2]-(lo[2]+hi[2])/2)*scale,(p[:,1]-lo[1])*scale,uv)).astype('<f4')
im=Image.open(io.BytesIO(view(j['images'][0]['bufferView']))).convert('RGBA');print('Texture',im.size,flush=True);im.thumbnail((2048,2048),Image.Resampling.LANCZOS);im.save(out/'texture.png')
assert len(p)<=300000 and count<=900000,(len(p),count)
with (out/'headquarters.xrhq').open('wb') as f:
 f.write(struct.pack('<6I',0x51485258,1,len(v),count,*im.size));f.write(v.tobytes());f.write(idx.astype('<u4').tobytes());f.write(im.tobytes())
np.savez(out/'preview.npz',vertices=v,indices=idx.reshape(-1,3))
report={'source':str(src),'source_triangles':source_triangles,'triangles':count//3,'vertices':len(v),'texture':im.size,'height':float(v[:,2].max()),'bytes':(out/'headquarters.xrhq').stat().st_size}
(out/'report.json').write_text(json.dumps(report,indent=2));print(report,flush=True)
