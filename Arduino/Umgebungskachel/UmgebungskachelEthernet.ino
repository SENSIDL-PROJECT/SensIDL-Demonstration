/**
    *** UmgebungskachelEthernet.ino ***
    This file holds all methods that are necessary for communicating with the Ethernet Port on the Arduino.
**/


/**
  This method reads a http request from a client and stops reading after the content is finished.
  @returns the Content of the request or an empty string if no content exists.
**/
String readClientRequest(EthernetClient client) {
  
  String request = "";
  String data = "";
  boolean content = false;
  int contentRemaining = 0;
  
  
  // an http request ends with a blank line
  while (client.connected()) {
    if (client.available()) {
          //read char
          char c = client.read();
          if(content) data +=c;
          else        request += c;
          
          //Header finished after two newlines
          if(request.endsWith("\r\n\r\n") && content == false){
            content = true;
            if(request.indexOf("Content-Length: ") > -1) { //If Content exists, read it
              String substr = request.substring(request.indexOf("Content-Length: ")+16);
              substr = substr.substring(0,substr.indexOf("\r\n"));
              //get Content-Length
              contentRemaining = substr.toInt();
            } else {
              break;
            }
          }
          
          if(content == true) {
           if (contentRemaining == 0) break;
            contentRemaining --;
          }
    } else {
      break;
    }
  }
  return data;
}

/**
  This method sends a standart http header to the given client.
**/
void sendHeader(EthernetClient client) {
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  // the connection will be closed after completion of the response
  client.println("Connection: close");
  client.println();  
}

/**
  getLocalIP reads the currentIP from the Ethernet API and formats it to fit a String.
**/
String getLocalIP() {
 char ip[24];
 IPAddress myIP = Ethernet.localIP(); 
 sprintf(ip,"%d.%d.%d.%d",myIP[0],myIP[1],myIP[2],myIP[3]);  
 return (String)ip;
}
