from distutils.core import setup, Extension
setup(name='gradspy', version='1.0',  \
      ext_modules=[Extension('gradspy', ['gradspy3.c'])])
