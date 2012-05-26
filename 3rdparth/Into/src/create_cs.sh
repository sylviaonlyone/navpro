CS_FILE=cscope.files;
CS_FILE_=cscope.files_;
INPUT_FILES="c h cpp hpp cc"
CSCOPE=cscope
CS_OUTPUT=cs.out

if [ -z $1 ]; then
  if [ -f $CS_OUTPUT ]; then
    rm -f $CS_OUTPUT
  fi
  #echo "- no out put file selected.";
  #echo "- usage: ./create_cs_files FILENAME.out";
  #exit 0;
else
  if [ -f $1 ]; then
    rm -f $1
  fi
  CS_OUTPUT=$1
fi

## remove the former exist CS_FILE
if [ -f $CS_FILE ]; then
  rm -f $CS_FILE
fi

if [ -f $CS_FILE_ ]; then
  rm -f $CS_FILE_
fi

## remove failed, echo and exit
if [ -f $CS_FILE ]; then
  echo "- $CS_FILE exist!!";
else
  echo "- creating $CS_FILE ...";

##
# there seems to be a bug:
# - it cannot do a recursive find nested in a for loop.
#
 for n in $INPUT_FILES; do
   # ! -type l : file type is not link
   # -a : and
   # >> doesn't truncate file, append on file
   find $PWD ! -type l -a -name "*.$n" >> $CS_FILE_;
  done
#  grep -v $PWD/include $CS_FILE_ | sort -u > $CS_FILE
#  grep $PWD/include $CS_FILE_ | grep -v '.h' | sort -u >> $CS_FILE
#  grep $PWD/include $CS_FILE_ | grep '.h' | grep '.hi' | sort -u >> $CS_FILE
#  rm -f $CS_FILE_

#find $PWD ! -type l -a -name "*.c" >> $CS_FILE_;
#find $PWD ! -type l -a -name "*.cpp" >> $CS_FILE_;
#find $PWD ! -type l -a -name "*.h" >> $CS_FILE_;
#find $PWD ! -type l -a -name "*.hpp" >> $CS_FILE_;
  cat $CS_FILE_ | sort -u > $CS_FILE;
  rm -f $CS_FILE_
fi

#if [ -f $1 ]; then
#echo "- $1 exist!! If you want to update the file, please remove it first.";
#else
if [ -f $CS_FILE ]; then
  echo "- creating $CS_OUTPUT ...";
  $CSCOPE -kbqR -i $CS_FILE -f $CS_OUTPUT;
  if [ -f $CS_OUTPUT ]; then
    echo "- success!!";
  else
    echo "- W: failed to generate $CS_OUTPUT!!";
  exit 1;
  fi
else
  echo "- W: failed to generate $CS_FILE!!";
  exit 1;
fi
#fi
## exit successfully!
exit 0;
