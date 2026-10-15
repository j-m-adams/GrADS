project = 'GrADS'
author = 'GrADS developers'
extensions = ['myst_parser']
templates_path = []
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
source_suffix = {'.md': 'markdown'}
root_doc = 'index'
html_theme = 'pydata_sphinx_theme'
html_title = 'GrADS Documentation'
html_static_path = ['_static']
html_css_files = ['custom.css']
html_theme_options = {
    'navigation_with_keys': True,
    'show_nav_level': 1,
    'navbar_end': ['theme-switcher', 'navbar-icon-links'],
}
myst_heading_anchors = 3
myst_enable_extensions = ['colon_fence', 'deflist', 'fieldlist', 'substitution']
