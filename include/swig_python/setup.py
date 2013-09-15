from distutils.core import setup, Extension

module1 = Extension('_pyccof',
                include_dirs=['../', '/usr/local/include','/usr/include/glib-2.0', 'usr/include/glib-2.0/glib', '/usr/lib/x86_64-linux-gnu/glib-2.0/include', '/usr/include/python2.7'],
                library_dirs=['/usr/lib', '/usr/lib/x86_64-linux-gnu', '/usr/local/lib'],
                runtime_library_dirs=['/lib/x86_64-linux-gnu', '/usr/lib', '/usr/lib/x86_64-linux-gnu', '/usr/local/lib'],
                libraries=['glib-2.0', 'python2.7', 'ccof'],
                language=['c'],
                sources=['./pyccof.i'])

setup(  name            = 'Ext_pyccof',
        version         = '1.0',
        author          = 'swiyer:CodeChix',
        description     = 'SWIG extended Py module - a generic openflow driver',
        license         = 'copyrighted@CodeChix Bay Area Chapter',
        platforms       = ['x86_64'],
        ext_modules     = [module1],
        py_modules      = ["pyccof"],
        )
