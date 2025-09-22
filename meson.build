project('ping-viewer', 'c')

gtkdep = dependency('gtk+-3.0')
sources = files(
  'src/main.c',
  'src/ping.c'
)

headers = files(
  'src/ping.h'
)

executable('demo', sources, dependencies : gtkdep)
