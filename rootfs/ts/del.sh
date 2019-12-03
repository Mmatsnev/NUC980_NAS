#!/bin/sh

#获取当前系统时间
curTimeSecs=`date +%s`

#删除时间间隔(秒)，目前测试用一天
delTimeSecs=`expr 24 \* 60 \* 60`

#执行删除的目录
dirPath="/mnt/mmcblk0p1/"

do_del() {
     for file in `ls $1`
     do
          tmp=""$1""$file""
          #echo "$tmp"
          if [ -d $tmp ]; then
               if [ `ls -A $tmp|wc -l` == "0" ]; then
                   fileCTimeSecs=`stat -c "%Z" $tmp`
                   if [ `expr $curTimeSecs - $fileCTimeSecs` -ge $delTimeSecs ]; then
                       rm -rf $tmp #删除目录
                   fi
               else
                   do_del "$tmp"/""
               fi
         else
               if [ "$tmp" != ""$dirPath""config"" ]; then 
                   fileCTimeSecs=`stat -c "%Z" $tmp`
                   if [ `expr $curTimeSecs - $fileCTimeSecs` -ge $delTimeSecs ]; then
                       rm $tmp  #删除文件
                   fi
               fi
         fi
     done
}

do_del $dirPath

