#!/usr/bin/python

import requests
import re
import serial
import time as sleepmodule
from time import strftime
from datetime import datetime, time

ser = serial.Serial('/dev/cu.usbserial-AL01C0MN',115200) #,bytesize=8, parity='N', stopbits=1)
startTime = datetime.now()
try:
        while True :
                line=ser.read(74)
                if any(c.isalpha() for c in line) == False:
                        continue
                else:
                        splitedline= line.split("STR:")
                        counterx = 0
                        for l in splitedline:
                                if counterx == 1:
                                        Node_ID1= l[0:6]
                                        l=Node_ID1
                                else: 
                                        if counterx == 2:
                                            Node_ID2= l[0:6]
                                            l=Node_ID2
                                        else: 
                                            if counterx == 3:
                                                Data= l[0:4]
                                                l=Data
                                print(l)
                                print("---------")
                                counterx+=1


                        r = requests.post('http://9.sensify-go.appspot.com/postsensorvalue?nodeid=18&sensor1value=Node_ID1&sensor2value=Node_ID2&sensor3value=Data&tagcode=952')
                        print (r.text)       
                                #Node_ID1= splitedline[0:6]
                        #print(Node_ID1)
                        #print("---------")
                        #print(Node_ID2)
                        #print("---------")
                        #print(Data)
                        #print("---------")
                        #now = datetime.now()
                        #post_time= now.strftime("%Y%m%d%H%M%S")
                        #sensor_ID="1"
                        #r = requests.post('http://192.168.22.160/api/createPressure.php', data = {'value':Node_ID1,'time':post_time,'sensor_ID':sensor_ID})
                        #print (r.text)
			#print(post_time)
                        #elapsedTime = now-startTime
                        #elapsedSeconds = (elapsedTime.microseconds+(elapsedTime.days*24*3600+elapsedTime.seconds)*10**6)/10**6
                        #print("%s,%s,%s"%(now.strftime("%Y-%m-%d %H:%M:%S"),elapsedSeconds,Node_ID1))
                        #f=open('tempLog.dat','a')
                        #print >>f,("%s,%s,%s"%(now.strftime("%Y-%m-%d %H:%M:%S"),elapsedSeconds,Node_ID1))
                        #f.close()
                        #sleepmodule.sleep(600)
except KeyboardInterrupt:
        print "close"





