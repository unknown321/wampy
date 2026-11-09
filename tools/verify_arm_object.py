#!/usr/bin/env python3
"""Fail closed on empty/truncated/wrong-architecture compiler output."""
import hashlib,json,pathlib,struct,subprocess,sys

def inspect(path,readelf='readelf'):
 p=pathlib.Path(path);data=p.read_bytes()
 if len(data)<52 or data[:7]!=b'\x7fELF\x01\x01\x01':raise ValueError(f'{p}: not a nonempty ELF32 little-endian object')
 fields=struct.unpack_from('<HHIIIIIHHHHHH',data,16)
 kind,machine=fields[:2];flags=fields[6];section_size,section_count,section_names=fields[10:13];section_offset=fields[5]
 if kind!=1 or machine!=40:raise ValueError(f'{p}: expected ARM ET_REL, got type={kind} machine={machine}')
 if not section_count or not section_size or section_offset+section_size*section_count>len(data):raise ValueError(f'{p}: truncated/absent section table')
 output={}
 for label,args in [('headers',['-h']),('attributes',['-A']),('sections',['-S']),('symbols',['-s'])]:
  r=subprocess.run([readelf,*args,str(p)],capture_output=True,text=True)
  if r.returncode or 'Error:' in r.stderr:raise ValueError(f'{p}: readelf {label}: {r.stderr}')
  output[label]=r.stdout
 if '.symtab' not in output['sections']:raise ValueError(f'{p}: missing symbol table')
 return {'path':str(p),'size':len(data),'sha256':hashlib.sha256(data).hexdigest(),'elf_type':'ET_REL','machine':'ARM','flags':hex(flags),'readelf':output}
if __name__=='__main__':
 try:print(json.dumps(inspect(sys.argv[1]),indent=2))
 except Exception as e:print(str(e),file=sys.stderr);sys.exit(1)
