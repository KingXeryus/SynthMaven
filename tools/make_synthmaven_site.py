#!/usr/bin/env python3
"""Build the SynthMaven editor + installer, without ever substituting an upstream binary."""
import argparse,json,shutil,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"web"))
import make_site
p=argparse.ArgumentParser();p.add_argument("--out",default="build/synthmaven-site");p.add_argument("--package");a=p.parse_args()
out=Path(a.out);web=Path(make_site.HERE);root=web.parent
if a.package:
 make_site.main(a.package,"0.3-dev",out)
else:
 inst=out/"webapp/installer";ed=out/"webapp/editor"
 inst.mkdir(parents=True,exist_ok=True);ed.mkdir(parents=True,exist_ok=True)
 lib="\n".join(make_site.strip_module((web/f).read_text()) for f in ("fm1pkg.js","fm1ota.js"))
 html=(web/"index_pkg.html").read_text().replace("/*LIB*/",lib).replace("/*META*/",json.dumps({"version":"0.3-dev","pkg":None,"product":None}))
 html=html.replace("<!--LOGO-->",(root/"assets/logo/synthmaven-logo.svg").read_text()).replace("<!--BANNER-->","<p><strong>Development preview: no firmware binary published.</strong></p>").replace("<!--STUDIO_GUIDE-->","")
 (inst/"index.html").write_text(html)
 shutil.copy(web/"editor.html",ed/"index.html")
 for f in ("fukiai.ttf","FUKIAI-LICENSE.txt"): shutil.copy(web/f,ed/f)
 shutil.copy(root/"assets/logo/synthmaven-logo.svg",ed/"synthmaven-logo.svg")
 (out/"index.html").write_text('<!doctype html><meta charset="utf-8"><title>SynthMaven</title><meta http-equiv="refresh" content="0;url=webapp/installer/">')
# corresponding source and licence notices, for every published site
src=out/"source";src.mkdir(exist_ok=True)
import zipfile
files=subprocess.check_output(["git","ls-files","--cached","--others","--exclude-standard","-z"],cwd=root).decode().split("\0")
with zipfile.ZipFile(src/"SynthMaven-source.zip","w",zipfile.ZIP_DEFLATED) as z:
 for name in files:
  if name and not name.startswith("build/") and (root/name).is_file(): z.write(root/name,"SynthMaven/"+name)
shutil.copy(root/"LICENSE",src/"LICENSE")
print(out)
