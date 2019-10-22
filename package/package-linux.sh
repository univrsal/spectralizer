#!/bin/sh
# Automatic packaging for linux builds

MACHINE_TYPE=`uname -m`
if [ ${MACHINE_TYPE} == 'x86_64' ]; then
	echo "Preparing 64bit build"
	bits="64"
else
	echo "Preparing 32bit build"
	bits="32"
fi

version=$1
data_dir="../data"
project="spectralizer"
arch="linux$bits"
build_location="../../../qtc-relwithdeb/rundir/RelWithDebInfo/obs-plugins/${bits}bit"
build_dir=$project.v$version.$arch

if [ -z "$version" ]; then
	echo "Please provide a version string"
	exit
fi

echo "Creating build directory"
mkdir -p $build_dir/plugin
mkdir -p $build_dir/plugin/bin/${bits}bit

echo "Fetching build from $build_location"
cp $build_location/$project.so $build_dir/plugin/bin/${bits}bit/

echo "Fetching locale from $data_dir"
cp -R $data_dir $build_dir/plugin

echo "Fetching misc files"
cp ../LICENSE $build_dir/LICENSE.txt
cp ./README.txt $build_dir/README.txt

echo "Writing version number $version"
sed -i -e "s/@VERSION/$version/g" $build_dir/README.txt

echo "Zipping to $project.v$version.$arch.zip"
cd $build_dir
zip -r "../$project.v$version.$arch.zip" ./ 
cd ..

echo "Cleaning up"
rm -rf $build_dir
