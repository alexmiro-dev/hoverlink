network_inc = include_directories('include')

network_sources = [
    'src/tcp_client.cpp',
    'src/tcp_server.cpp',
    'src/udp_client.cpp',
]

network_lib = library(
    'hoverlink-network',
    network_sources,
    include_directories : network_inc,
    dependencies : [boost_dep],
    install : true
)

network_dep = declare_dependency(
    include_directories : network_inc,
    link_with : network_lib
)