from distutils.core import setup, Extension
setup(name='gradspy', version='1.1',  \
      ext_modules=[Extension('gradspy', ['gradspy.c'])])
