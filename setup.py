import os
import subprocess
import sys
from setuptools import setup, Extension, find_packages
from setuptools.command.install import install
from setuptools.command.build_ext import build_ext

class CMakeBuild(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}"
        ]
        build_temp = self.build_temp
        if not os.path.exists(build_temp):
            os.makedirs(build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=build_temp)
        subprocess.check_call(['cmake', '--build', '.'], cwd=build_temp)

class InstallWithCMake(install):
    def run(self):
        self.run_command('build_ext')
        install.run(self)

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        super().__init__(name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="simevopy",
    version='0.2.26',
    description="SimEvo Python bindings",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=find_packages(),
    ext_modules=[CMakeExtension('simevopy')],
    cmdclass=dict(build_ext=CMakeBuild, install=InstallWithCMake),
    url='https://github.com/YJack0000/SimEvo',
    author='YJack0000',
    author_email='yjack0000.cs12@nycu.edu.tw',
    license='MIT',
)
