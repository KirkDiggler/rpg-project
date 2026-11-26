import json, os, re, shlex, subprocess, sys, time
P=os.environ.get('RPGUE_PROJECT', os.path.expanduser('~/UnrealEngine/Projects/RpgUeProof'))
d=json.load(open(P+'/.vscode/compile_commands.json'))
plain=lambda s: re.sub(r'\x1b\[[0-9;]*m', '', s)
bad=0
for e in d:
    if '/Intermediate/' in e['file'] or '.pb.' in e['file']: continue
    cmd=e.get('arguments') or shlex.split(e['command'])
    t=time.time()
    r=subprocess.run(cmd+['-fsyntax-only', '-Qunused-arguments'], cwd=e['directory'], capture_output=True, text=True)
    err=[l for l in plain(r.stderr).splitlines() if ': error: ' in l]
    short=e['file'].split('/Source/')[-1]
    if r.returncode: bad+=1
    print('  %-50s %s %d error(s) %.0fs' % (short, 'ok ' if r.returncode==0 else 'FAIL', len(err), time.time()-t))
    for l in err[:int(sys.argv[1]) if len(sys.argv)>1 else 0]: print('       '+l.split('/Projects/RpgUeProof/')[-1][:200])
sys.exit(1 if bad else 0)
