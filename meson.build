project('ping-viewer', 'c')

gtkdep = dependency('gtk4')
sources = files(
  'src/conf.c',
  'src/list.c',
  'src/host.c',
  'src/ping-viewer.c',
  'src/ping.c'
)

executable('ping-viewer', sources, dependencies : gtkdep)
