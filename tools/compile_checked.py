#!/usr/bin/env python3
"""Compiler command must use {output}; only verified objects reach final paths."""
import json,os,pathlib,subprocess,sys,tempfile
from verify_arm_object import inspect

def compile_checked(final,command,log,readelf='readelf'):
 final=pathlib.Path(final).resolve();final.parent.mkdir(parents=True,exist_ok=True)
 fd,tmp=tempfile.mkstemp(prefix=final.name+'.pending-',suffix='.o',dir=final.parent);os.close(fd)
 cmd=[part.replace('{output}',tmp) for part in command]
 if tmp not in cmd:raise ValueError('Compiler command must contain a standalone {output} argument')
 r=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 pathlib.Path(log).write_text(json.dumps(cmd)+'\n'+r.stdout+'\nexit='+str(r.returncode)+'\n')
 if r.returncode:os.unlink(tmp);raise RuntimeError(f'Compiler failed: {r.returncode}')
 try:result=inspect(tmp,readelf)
 except Exception:
  os.unlink(tmp);raise
 with open(tmp,'rb') as f:os.fsync(f.fileno())
 os.replace(tmp,final)
 d=os.open(final.parent,os.O_RDONLY);os.fsync(d);os.close(d)
 published=inspect(final,readelf)
 if published['sha256']!=result['sha256']:raise RuntimeError('Published object hash changed')
 result=published
 result['path']=str(final)
 pathlib.Path(str(final)+'.verified.json').write_text(json.dumps(result,indent=2))
 return result
if __name__=='__main__':
 # input JSON: final, command array, log; avoids shell expansion.
 request=json.load(sys.stdin)
 result=compile_checked(**request);print(json.dumps({k:v for k,v in result.items() if k!='readelf'},indent=2))
