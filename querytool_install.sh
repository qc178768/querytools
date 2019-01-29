#!/bin/bash
pwd=`pwd`
cd $pwd/createindex/source/
./install.sh
if [ $? -eq 0 ]; then
	echo "createindex install success" > $pwd/install.log
else
	echo "createindex install failure" > $pwd/install.log
	exit 1
fi	
cd $pwd/cdrquery/source/
./install.sh
if [ $? -eq 0 ]; then
	echo "query install success" >> $pwd/install.log
else
	echo "query install failure" >> $pwd/install.log
	exit 1
fi


