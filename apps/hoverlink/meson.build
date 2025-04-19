hoverlink_inc = include_directories('include')

hoverlink_src = [
    'src/main.cpp',
]

executable('hoverlink',
           hoverlink_src,
           include_directories : hoverlink_inc,
           dependencies : [
               protocol_dep,
               boost_dep
           ],
           install : false
)