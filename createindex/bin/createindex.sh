#!/bin/sh
source /opt/utoss/querytools/createindex/bin/dirs.sh
echo "run createindex.sh"
if [ "$dirs" != "" ]; then
    for i in $dirs; do
		if [ -d "$i" ];then
			/opt/utoss/querytools/createindex/bin/createindex -d $i 
		fi
		#echo $i
    done
fi
