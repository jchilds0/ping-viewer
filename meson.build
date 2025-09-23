project('ping-viewer', 'c')

gtkdep = dependency('gtk4')
sources = files(
  'src/main.c',
  'src/list-host.c',
  'src/ping-host.c',
  'src/ping.c'
)

executable('demo', sources, dependencies : gtkdep)
