#! /bin/bash
# install.sh re-implement of install.bat

into_dir=~/tools/Into/src

for file in $( find $PWD -name '*.so')
do
echo $file
#ln -s $file $into_dir/lib/ 
sudo cp $file /usr/lib
done
exit $
