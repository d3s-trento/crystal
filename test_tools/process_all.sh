#!/bin/bash
TOOLPATH=`dirname $0`

TESTBED=`head -n1 testbed.txt`

if [ $TESTBED = indriya ]
then
	EXT=.zip
elif [ $TESTBED = fbk ]
then
	EXT=.tar.gz
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
fi
}

shopt -s nullglob

for x in */data*$EXT
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
		#sed -e "s/\\\0//g" -e "s/\\\n/	/g" < *.dat > log.cleaned
		cat *.dat | sed -e "s/\\\n\(\\\0\)*//g" > log.cleaned
		rm -f *.dat messages.pickle.gz
		if [ $TESTBED = indriya ]
		then
			python ../../$TOOLPATH/parser_short.py --format $TESTBED
			Rscript ../../$TOOLPATH/stats_short.R > stats_out.txt
		else
			python ../../$TOOLPATH/parser_ta.py --format $TESTBED
			Rscript ../../$TOOLPATH/stats_ta.R > stats_out.txt
			#Rscript ../../$TOOLPATH/plot_all_pernode.R
			grep OLD_PDR stats_out.txt
			grep "Max starting epoch" stats_out.txt
		fi
		#rm -f log.cleaned
		cd ../..
	fi
done
