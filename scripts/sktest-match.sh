#!/bin/sh

USAGE="Usage: $0 [-memcheck]"

if test $# -eq 0
then
  MC=""
else
  if test $# -eq 1
  then
    if test "$1" = "-memcheck"
    then
      MC="-memcheck" 
    else
      echo ${USAGE}
      exit 1
    fi 
  else
    echo ${USAGE}
    exit 1
  fi
fi

cerr()
{
  $*
  if [ $? -ne 0 ]
  then
    echo "failure: $*"
    exit 1
  fi
}

cd testsuite

for keyword in gt_idxlocali gt_greedyfwdmat gt_tallymer
do
  env -i GT_MEM_BOOKKEEPING=on ./testsuite.rb ${MC} -keywords ${keyword}
  if test $? -ne 0
  then
    exit 1
  fi
done

cd ..

for inputfile in `ls testdata/*.fna` ${AT} ${U8} ${ATK} `ls ${GTTESTDATA}/DNA-mix/Grumbach.fna/*.fna`
do
  if test ${inputfile} = 'testdata/TTT-small.fna'
  then
    echo "skip ${inputfile}"
  else
    if test ${inputfile} = 'testdata/Random-Small.fna'
    then
      echo "skip ${inputfile}"
    else
      echo "${inputfile}"
      cerr "scripts/checktallymer.sh ${inputfile}"
    fi
  fi
done

