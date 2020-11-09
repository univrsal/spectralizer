#!/bin/sh

echo "Checking for brew..."
if ! [ -x "$(command -v brew)" ]; then
	echo 'Error: brew is not installed!'
	echo 'Install brew from https://brew.sh'
	exit 1
fi

echo "Checking for fftw..."
if brew ls --versions fftw > /dev/null; then
	echo "fftw is already installed"
else
	echo "Installing fftw"
	brew install fftw
fi

plugin_path="${HOME}/Library/Application Support/obs-studio/plugins/spectralizer"

echo "Uninstalling old version"
rm -rf "${plugin_path}"

echo "Installing plugin"
(
	cd plugin || exit
	for dir in *; do
		mkdir -p "${plugin_path}/${dir}"
		cp -R "${dir}"/* "${plugin_path}/${dir}/"
	done
)

echo "Bypassing Gatekeeper for the plugin"
sudo xattr -rd com.apple.quarantine "${plugin_path}/bin/spectralizer.so"

echo "Done!"
exit 0
