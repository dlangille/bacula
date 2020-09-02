import os, sys

scripts_dir=''
# search the "scripts" directory with blab.py trying from CWD and "this" __file__
for base in [ os.path.abspath('scripts'), os.path.join(os.path.dirname(__file__), 'scripts') ]:
    if not os.path.isdir(base):
        continue
    if os.path.isfile(os.path.join(base, 'blab.py')):
        sys.path.insert(0, base)
        import blab
        break
else:
    print('blab.py not found')

