project('idkWM','cpp')

cpp_files = [
'main.cpp',
'idkWM/idkWM.cpp',
'idkWM/events.cpp',
'idkWM/log.cpp',
'../lib/json.cpp',
'idkWM/config_parser.cpp',
'idkWM/tiling.cpp'

]
run_target('run',
  command : 'run.sh')

cpp_args = ['-Os','-Wall','-Wextra','-Werror','-lX11']
link_args = ['-lX11']

executable('idkWM',cpp_files,c_args: cpp_args,link_args: link_args)
