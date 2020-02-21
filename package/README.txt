spectralizer v1.2
This program is licensed under the GPL v2.0 See LICENSE.txt
github.com/univrsal/spectralizer

INSTALLATION

macOS:
------
Either run install-mac.sh or follow these steps:

0. Install fftw3 over brew
   $ brew install fftw
1. Create a folder for the plugin:
   $ mkdir -p "/Users/$USER/Library/Application Support/obs-studio/plugins/spectralizer"
2. Copy over the folders "bin" and "data" from the folder "plugin" from this zip file:
   $ mv plugin/* "/Users/$USER/Library/Application Support/obs-studio/plugins/spectralizer"

Linux:
------
0. Install fftw3 over your package manager
1. Create a plugin folder in your home directory:
  $ mkdir -p ~/.config/obs-studio/plugins/spectralizer
2. Extract the bolder bin and data into the newly created folder
  $ mv plugin/* ~/.config/obs-studio/plugins/spectralizer

Windows:
--------
1. Extract the archive
2. Move the contents of plugin into your obs installation directory
