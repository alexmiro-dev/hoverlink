protocol_inc = include_directories('include')

# Define Flatbuffer schemas
flatbuffer_schemas = [
    'schemas/command.fbs',
    'schemas/status.fbs',
    'schemas/telemetry.fbs',
]

flatc = find_program('flatc')

# Generate headers from Flatbuffer schemas
flatbuffer_gen = []
foreach schema : flatbuffer_schemas
    schema_path = join_paths(meson.current_source_dir(), schema)
    output_name = '@BASENAME@_generated.h'
    target = custom_target(
        schema.underscorify() + '_gen',
        input : schema_path,
        output : output_name,
        command : [flatc, '--cpp', '--gen-mutable', '--scoped-enums', '--gen-object-api',
                   '-o', '@OUTDIR@', '@INPUT@'],
        build_by_default : true
    )
    flatbuffer_gen += target
endforeach

# Declare generated include directory.  Crucially, do this *after*
# the custom_targets are defined.
generated_inc_dir = include_directories('.')

# Source files
protocol_sources = [
    'src/messages.cpp',
]

flatbuffers_dep = dependency('flatbuffers')

protocol_lib = library(
    'hoverlink-protocol',
    protocol_sources + flatbuffer_gen,  # Add generated files to sources
    dependencies : [flatbuffers_dep],   # Only keep actual dependencies here
    include_directories : [protocol_inc, generated_inc_dir],
    install : false
)

protocol_dep = declare_dependency(
    include_directories : [protocol_inc, generated_inc_dir],
    link_with : protocol_lib,
    dependencies : [flatbuffers_dep],
    sources : flatbuffer_gen  # You had this commented out, but it should be included
)