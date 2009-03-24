- The RTSP Source filter uses the open source LGPL liveMedia RTSP/RTP/RTCP to handle the streaming of media. 
You can download it at www.live555.com.

- Running an RTSP Server:
To test this filter you can download the RTSP media server from www.live555.com. 
Copy a wav file into the server directory and start the RTSP server.

- Registering the client:
Run the rtsp.reg file in the RTVC\Projects\Win32\Launch directory.

- Listening to the stream:
Open Windows Media Player
Select File->Open Url from the menu
Enter the RTSP URL of the webserver: rtsp://xxx.xxx.xxx.xxx/filename.wav
