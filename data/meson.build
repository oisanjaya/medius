gresource = gnome.compile_resources(
  'medius-res',
  'resources.xml',
  source_dir: meson.current_source_dir(),
  c_name: 'medius'
)

install_data('medius.ui', install_dir: join_paths(get_option('datadir'), 'medius'))
install_data('icons/app-icon.png', install_dir: join_paths(get_option('datadir'), 'medius/icons'))
