from setuptools import setup, Extension

hashtable_module = Extension(
    "hashtable",
    sources=["hashtable_wrapper.c", "hashtable.c", "fnv.c"],
    include_dirs=["."],
)

setup(
    name="hashtable",
    version="1.0",
    description="Python wrapper for hashtable library",
    ext_modules=[hashtable_module],
)
