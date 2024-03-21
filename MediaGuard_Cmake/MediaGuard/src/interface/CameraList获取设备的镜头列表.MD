# API : /{Language}/Cam/CameraList  :  CameraList获取设备的镜头列表

# Request:

{
    "maincomId":"6000014",
    "deviceId":"3007"
}
   
Bearer Token : xxxxxxxxxxxxxxxxxx

   
# Response:
   
{  
    "meta": {  
        "success": true,  
        "message": "OK",  
        "errorCode": -1  
    },  
    "data": [  
        {  
            "rtspIp": "192.168.0.92",  
            "rtspUsername": "admin",  
            "rtspPassword": "hik12345",  
            "maincomId": "6000014",  
            "cameraId": 8,  
            "name": "攝像鏡頭19",  
            "cameraIp": null,  
            "siteId": 10,  
            "siteName": "6樓",  
            "rtsp": "rtsp://admin:hik12345@192.168.0.92:554/h264/ch1/main/av_stream",  
            "type": 0,  
            "online": "使用中",  
            "remark": "Floor3  3006",  
            "createTime": "2023-01-20 00:00:00",  
            "deviceId": 3007,  
            "deviceName": "HUA_SERVER1",  
            "recordStatus": 0,  
            "deviceType": 1006,  
            "onlive": true  
        },  
        {  
            "rtspIp": "192.168.0.91",  
            "rtspUsername": "admin",  
            "rtspPassword": "admin888888",  
            "maincomId": "6000014",  
            "cameraId": 12,  
            "name": "大華IP91",  
            "cameraIp": null,  
            "siteId": 12,  
            "siteName": "7樓A座",  
            "rtsp": "rtsp://admin:admin888888@192.168.0.91:554/cam/realmonitor?channel=1&subtype=0",  
            "type": 0,  
            "online": "使用中",  
            "remark": "大華鏡頭IP91",  
            "createTime": "2023-01-20 00:00:00",  
            "deviceId": 3007,  
            "deviceName": "HUA_SERVER1",  
            "recordStatus": 1,  
            "deviceType": 1006,  
            "onlive": true  
        }  
    ]  
}  