#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>   
#include<unistd.h>  
#include<iostream>
#include<fstream>
#include<errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "ThreadSafeDetector.h"
#include "tao/json.hpp"
#include "BoxSerializer.h"
#include <chrono>

using namespace std;

// Get current date/time, format is YYYY-MM-DD|HH:mm:ss
const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
    return buf;
}


/**
    Send data to the connected host
*/
bool send_data(int socket,std::string data)
{
    //Send some data
    if( send(socket , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    cout<<"Data send\n";
     
    return true;
}
 
/**
    Receive data from the connected host
*/
std::string receive(int socket,int size=512)
{
    char buffer[size];
    std::string reply;
     
    //Receive a reply from the server
    if( recv(socket , buffer , sizeof(buffer) , 0) < 0)
    {
        puts("recv failed");
    }
     
    reply = buffer;
    return reply;
}

cv::Mat fromData(const std::vector<unsigned char>& data)
{
 cv::Mat tmp(data);
 cv::Mat img = cv::imdecode(tmp, 1);
 return img;
}

std::string receive_image(int socket,std::shared_ptr<ThreadSafeDetector> detector)
{ 
    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
    
    char imagearray[10241],verify = '1';
    FILE *image;
    
    //Find the size of the image
    do{
    stat = read(socket, &size, sizeof(int));
    }while(stat<0);
    
    printf("Packet received.\n");
    printf("Packet size: %i\n",stat);
    printf("Image size: %i\n",size);
    printf(" \n");
    
    char buffer[] = "Got it";
    
    //Send our verification signal
    do{
    stat = write(socket, &buffer, sizeof(int));
    }while(stat<0);
    
    printf("Reply sent\n");
    printf(" \n");
    char fileName[50];
    sprintf(fileName,"Image/%s.jpeg",currentDateTime().c_str());
    image = fopen(fileName, "w");
    
    if( image == NULL) {
    printf("Error has occurred. Image file could not be opened\n");
    return ""; }
    
    //Loop while we have not received the entire file yet
    
    
    int need_exit = 0;
    struct timeval timeout = {10,0};
    
    fd_set fds;
    int buffer_fd, buffer_out;
    std::vector<unsigned char> img;
    while(recv_size < size) {
    //while(packet_index < 2){
    
        FD_ZERO(&fds);
        FD_SET(socket,&fds);
    
        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);
    
        if (buffer_fd < 0)
           printf("error: bad file descriptor set.\n");
    
        if (buffer_fd == 0)
           printf("error: buffer read timeout expired.\n");
    
        if (buffer_fd > 0)
        {
            do{
                   read_size = read(socket,imagearray, 10241);
                }while(read_size <0);
    
             std::vector<unsigned char> tmp(imagearray,imagearray + read_size);
             img.insert(std::end(img), std::begin(tmp), std::end(tmp));       

            printf("Packet number received: %i\n",packet_index);
            printf("Packet size: %i\n",read_size);
    
    
            //Write the currently read data into our image file
             write_size = fwrite(imagearray,1,read_size, image);
             printf("Written image size: %i\n",write_size); 
    
                 if(read_size !=write_size) {
                     printf("error in read write\n");    
                  }
        
                 //Increment the total number of bytes read
                 recv_size += read_size;
                 packet_index++;
                 printf("Total received image size: %i\n",recv_size);
                 printf(" \n");
                 printf(" \n");
        }
    
    }    
    
      fclose(image);
      printf("Image successfully Received!\n");

      cv::Mat matImage = fromData(img);

      if( matImage.empty() )// Check for invalid input
      {
          cout <<  "Could not open or find the image" << std::endl ;
          return "";
      }
      std::vector<bbox_t>boxes;
      std::vector<cv::Rect>v_rect;
        auto begin = std::chrono::steady_clock::now();
      boxes = detector->detect(matImage);
        auto end = std::chrono::steady_clock::now();
  
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "The time: " << elapsed_ms.count() << " ms\n";

      std::string stringForSend=BoxSerializer::toString(boxes);

      for(auto box:boxes)   
      {    
        v_rect.push_back(cv::Rect(box.x,box.y,box.w,box.h));
      }
      std::cout <<"\nTry to send string\n";

      send_data(socket,stringForSend);

      return stringForSend;
}

int main(int argc , char *argv[])
{
  const std::string DARKNET_CONFIG_PATH = "data/yolov3-tiny_obj.cfg";
  const std::string DARKNET_WEIGHTS_PATH = "data/yolov3-tiny_obj_19200.weights";
  std::shared_ptr<ThreadSafeDetector> detector =
     std::make_shared<ThreadSafeDetector>(DARKNET_CONFIG_PATH, DARKNET_WEIGHTS_PATH, 0);

  int socket_desc , new_socket , c, read_size,buffer = 0;
  struct sockaddr_in server , client;
  char *readin;
  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
     printf("Could not create socket");
  }
  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8889 );
    //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
    puts("bind failed");
    return 1;
  }
  puts("bind done");
  while(1)
  {
    //Listen
    listen(socket_desc , 3);
     //Accept and incoming connection
     puts("Waiting for incoming connections...");
     c = sizeof(struct sockaddr_in);
    if((new_socket = accept(socket_desc, (struct sockaddr *)&client,(socklen_t*)&c)))
    {
      puts("Connection accepted");
    }
  
    fflush(stdout);
  
    if (new_socket<0)
    {
      perror("Accept Failed");
      return 1;
    }
  
    std::string stringJSON = receive_image(new_socket,detector);
    std::cout << stringJSON<<std::endl;
    
  }

  close(socket_desc);
  fflush(stdout);
  return 0;
}