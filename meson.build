project('ping-viewer', 'c')

gtkdep = dependency('gtk4')
sources = files(
  'src/ping-viewer.c',
  'src/list.c',
  'src/host.c',
  'src/ping.c'
)

executable('ping-viewer', sources, dependencies : gtkdep)
