#! /bin/bash
# remove .so files from /use/lib, remove debug and release folders in src directory

into_dir=$PWD

for file in $( find $PWD -name '*.so')
do
echo $file
#ln -s $file $into_dir/lib/ 
sudo rm -f /usr/lib/$file
done
find . -depth -name 'debug' -exec rm -rf '{}' \; -print
find . -depth -name 'release' -exec rm -rf '{}' \; -print
#rm -rf 'find . -type d -name debug'
#rm -rf 'find . -type d -name release'
exit $
