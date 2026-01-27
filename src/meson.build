add_project_arguments(
    [
      '-DDEFAULT_PANEL_WIDTH=150',
      '-DDEFAULT_PANEL_MARGIN=0',
      '-DDEFAULT_PANEL_PADDING=10',
      '-DDEFAULT_LIST_ITEM_BOX_HOVER_COLOR="#00000000"',
      '-DDEFAULT_ICON="dialog-error"',
      '-DDEFAULT_SLIDER_TIMEOUT=500',
    ],
    language: 'cpp'
)

executable('medius',
  [
    'helper/globals.cc', 'main.cc', 'config/config.cc', 'config/row_item.cc', 
    'helper/widget_css_provider.cc',
    'widgets/expander_item.cc', 'widgets/base_widget.cc', 'widgets/button_widget.cc', 
    'widgets/list_widget.cc', 'widgets/slider_widget.cc', 'widgets/image_widget.cc',
     'widgets/popover_widget.cc', 'widgets/textbox_widget.cc', 'window.cc', gresource
  ],
  dependencies: [
    ckdl_dep,
    gtk_dep, 
    spdlog_dep,
    gtk4_layer_shell_dep
  ],
  include_directories: inc_dirs,
  install: true
)