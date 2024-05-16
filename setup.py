from skbuild import setup
from setuptools import find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="simevopy",
    version="0.1.1",
    description="SimEvo Python bindings",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=find_packages(),
    cmake_source_dir=".",
    url='https://github.com/YJack0000/SimEvo',
    author='YJack0000',
    author_email='yjack0000.cs12@nycu.edu.tw',
    license='MIT',
)
