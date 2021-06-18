#!/usr/bin/env python

import os
from setuptools import setup
from setuptools.command.install import install as _install
from setuptools.extension import Extension
from distutils.spawn import find_executable
from Cython.Distutils import build_ext

ext_modules = [
    Extension('cams',
              sources=['cams/pycams.pyx','cams/offline_cams.c','cams/offline_revolve.c'],
              language='c'
             ),
    Extension('ac',
              sources=['cams/pyca.pyx','cams/offline_cams.c','cams/offline_revolve.c'],
              language='c'
             ),
]

#def build(dry_run=False):
#    if dry_run: return
#    make = find_executable('make')
#    command = [make, 'lib']
#    status = os.system(" ".join(command))
#    if status != 0: raise RuntimeError(status)

#class cmd_install(_install):
#    def run(self):
#        build(self.dry_run)
#        _install.run(self)

setup(
    name='cams',
    zip_safe=False,
    version='0.1',
    author='Hong Zhang',
    author_email='hongzhang@anl.gov',
    description='Checkpointing for adjoint multistage time stepping schemes',
    ext_modules=ext_modules,
    cmdclass={'build_ext':build_ext},
)
