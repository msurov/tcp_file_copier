#!/bin/bash

src="/tmp/received_files/src"
storage="/tmp/received_files/dst"
port=16010

nfiles=1000


mkdir -p ${src} 1>/dev/null 2>/dev/null
mkdir -p ${storage} 1>/dev/null 2>/dev/null
rm -f ${src}/* 1>/dev/null 2>/dev/null
rm -f ${storage}/* 1>/dev/null 2>/dev/null

killall receive_srv 1>/dev/null 2>/dev/null
killall send_files 1>/dev/null 2>/dev/null

printf "storage=${storage}\nport=${port}\n" > server_config.txt
printf "server=::1\nport=${port}\n" > client_config.txt


####################################################
## TEST 1
##  send 100 files by one client asynchronously
####################################################

for (( i=0; i < ${nfiles}; i++ ));
do
	$(dd if=/dev/urandom of=${src}/a$i.tmp bs=1K count=16 1>/dev/null 2>/dev/null) 
done

# run server and client
./receive_srv 1>/dev/null 2>/dev/null &
sleep 1
./send_files ${src}/* 1>/dev/null 2>/dev/null
sleep 1

# close server
killall receive_srv 

#compare files
diff -r ${src} ${storage}
RESULT=$?

if [ $RESULT -eq 0 ];then
   echo "[test 1] ok"
else
   echo "[test 1] fail!"
fi

rm -f ${src}/* 1>/dev/null 2>/dev/null
rm -f ${storage}/* 1>/dev/null 2>/dev/null


####################################################
## TEST 2
## send 100 files by 100 clients
####################################################
for (( i=0; i < ${nfiles}; i++ ));
do
	$(dd if=/dev/urandom of=${src}/b$i.tmp bs=1K count=16 1>/dev/null 2>/dev/null) 
done

# run server
./receive_srv 1>/dev/null 2>/dev/null &
sleep 1

# and 100 clients
for (( i=0; i < ${nfiles}; i++ ));
do
	./send_files ${src}/b$i.tmp 1>/dev/null 2>/dev/null &
done
sleep 1

# close server
killall receive_srv 

#compare files
diff -r ${src} ${storage}
RESULT=$?

if [ $RESULT -eq 0 ];then
   echo "[test 2] ok"
else
   echo "[test 2] fail!"
fi

rm -f ${src}/* 1>/dev/null 2>/dev/null
rm -f ${storage}/* 1>/dev/null 2>/dev/null
