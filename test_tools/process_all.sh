#!/bin/bash
TOOLPATH=`dirname $0`

echo Base path: $TOOLPATH

TESTBED=`head -n1 testbed.txt`

if [ $TESTBED = indriya ]
then
	EXT=.zip
elif [ $TESTBED = fbk ]
then
	EXT=.tar.gz
elif [ $TESTBED = twist ]
then
	EXT=.txt.gz
else
    echo "Unknown testbed"
	exit 1
fi

uncompress() {
if [ $TESTBED = indriya ]
then
    DIRNAME="${1%%.zip}"
    mkdir $DIRNAME
    unzip -d $DIRNAME $1
elif [ $TESTBED = fbk ]
then
	tar xvzfk $1
elif [ $TESTBED = twist ]
then
    DIRNAME="${1%%.txt.gz}"
    mkdir $DIRNAME
    zcat $1 > $DIRNAME/raw.log
fi
}

preparse() {
if [ $TESTBED = indriya ]
then
  cat *.dat | sed -e "s/\\\n\(\\\0\)*//g" > log.cleaned
  rm -f *.dat messages.pickle.gz
elif [ $TESTBED = fbk ]
then
  cat *.dat | sed -e "s/\\\n\(\\\0\)*//g" > log.cleaned
  rm -f *.dat messages.pickle.gz
elif [ $TESTBED = twist ]
then
  $TOOLPATH/twist_parser.py < raw.log > log.cleaned
fi
}


shopt -s nullglob

for x in */*$EXT
do
	DATA_DIR=${x%%$EXT}
	TOP_DIR=`dirname $x`
	ARCHIVE=`basename $x`
	if [ ! -e $DATA_DIR ]
	then
		echo Processing: $x
		cd $TOP_DIR
		uncompress $ARCHIVE 
		cp -v params_tbl.txt `basename $DATA_DIR` 
		cd `basename $DATA_DIR`
                preparse
		if [ $TESTBED = indriya ]
		then
			python $TOOLPATH/parser_short.py --format $TESTBED
			Rscript $TOOLPATH/stats_short.R > stats_out.txt
		else
			$TOOLPATH/parser_ta.py --format $TESTBED
			Rscript $TOOLPATH/stats_ta.R > stats_out.txt
			grep OLD_PDR stats_out.txt
			Rscript $TOOLPATH/plot_all_pernode.R
			grep "Max starting epoch" stats_out.txt
		fi
		#rm -f log.cleaned
		cd ../..
	fi
done
