EXPDIR=$PWD
for x in `find -name data-\* -type d`; do echo $x; cd $x; (Rscript $EXPDIR/../../test_tools/stats_ta.R > stats_out.txt); cd -; done
