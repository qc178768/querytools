#!/bin/sh
grep  "createindex.sh" /etc/crontab
if [ $? -ne 0 ]; then
	echo  '*/5 *  * * *  root      /opt/utoss/querytools/createindex/bin/createindex.sh' >> /etc/crontab
fi
