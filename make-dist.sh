#!/bin/sh
VER=`svnversion -cn trunk`
REV=`echo "$VER" | cut -d: -f 2`
DIR=btanks-0.4.$REV
echo "revision $REV"
echo "cleaning up..."
rm -rf ../$DIR
echo "exporting..."
svn export trunk $DIR
echo -n $VER > $DIR/.svnversion
rm -f $DIR/make-dist.sh
