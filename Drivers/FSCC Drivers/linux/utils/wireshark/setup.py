import sys
import os

from cx_Freeze import setup, Executable

if os.name == 'nt':
    cfscc_path = (os.path.join(sys.prefix, 'DLLs\cfscc.dll'), 'cfscc.dll')
else:
    cfscc_path = (os.path.join(sys.prefix, 'DLLs\libcfscc.so'), 'libcfscc.so')

buildOptions = dict(packages = ['fscc'], excludes = [], includes = ['re'],
                    include_files = [cfscc_path])

executables = [
    Executable('wfscc.py')
]

setup(name='wfscc',
      version = '1.1.0',
      options = dict(build_exe = buildOptions),
      executables = executables)
