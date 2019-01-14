#include <fcntl.h>  
#include <stdlib.h>  
#include <sys/mman.h>  
#include <linux/videodev2.h>  
  
int main(){  
  //////  
  int fd = open("/dev/video0",O_RDWR);  
  printf("TK------->>>fd is %d\n",fd);  
  //////  
  struct v4l2_capability cap;  
  ioctl(fd,VIDIOC_QUERYCAP,&cap);  
  printf("TK---------->>>>>Driver Name:%s\nCard Name:%s\nBus info:%s\n",cap.driver,cap.card,cap.bus_info);  
  //////  
  struct v4l2_fmtdesc fmtdesc;  
  fmtdesc.index = 0; fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
  while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){  
   printf("TK-------->>>>>fmtdesc.description is %s\n",fmtdesc.description);  
   fmtdesc.index ++;  
  }  
  //////  
  struct v4l2_format fmt;  
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
  ioctl(fd,VIDIOC_G_FMT,&fmt);  
  printf("TK----------->>>>>fmt.fmt.width is %d\nfmt.fmt.pix.height is %d\nfmt.fmt.pix.colorspace is %d\n",fmt.fmt.pix.width,fmt.fmt.pix.height,fmt.fmt.pix.colorspace);  
  //////  
  struct v4l2_requestbuffers req;  
  req.count = 4;  
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
  req.memory = V4L2_MEMORY_MMAP;  
  ioctl(fd,VIDIOC_REQBUFS,&req);  
  struct buffer{  
    void *start;  
    unsigned int length;  
  }*buffers;  
  buffers = (struct buffer*)calloc (req.count, sizeof(*buffers));  
  unsigned int n_buffers = 0;  
  for(n_buffers = 0; n_buffers < req.count; ++n_buffers){  
    struct v4l2_buffer buf;  
    memset(&buf,0,sizeof(buf));  
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    buf.memory = V4L2_MEMORY_MMAP;  
    buf.index = n_buffers;  
    if(ioctl(fd,VIDIOC_QUERYBUF,&buf) == -1){  
      printf("TK---------_>>>>>>error\n");  
      close(fd);  
      exit(-1);  
    }  
    buffers[n_buffers].length = buf.length;  
    buffers[n_buffers].start = mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);  
    if(MAP_FAILED == buffers[n_buffers].start){  
      printf("TK--------__>>>>>error 2\n");  
      close(fd);  
      exit(-1);  
    }  
  }  
  ////  
  unsigned int i;  
  enum v4l2_buf_type type;  
  for(i = 0; i < 4; i++){  
    struct v4l2_buffer buf;  
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    buf.memory = V4L2_MEMORY_MMAP;  
    buf.index = i;  
    ioctl(fd,VIDIOC_QBUF,&buf);  
  }  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
  ioctl(fd,VIDIOC_STREAMON,&type);  
  ////  
  struct v4l2_buffer buf;  
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
  buf.memory = V4L2_MEMORY_MMAP;  
  ioctl(fd,VIDIOC_DQBUF,&buf);  
  char path[20];  
  snprintf(path,sizeof(path),"./yuyv%d",buf.index);  
  int fdyuyv = open(path,O_WRONLY|O_CREAT,00700);  
  printf("TK--------->>>>fdyuyv is %d\n",fdyuyv);  
  int resultyuyv = write(fdyuyv,buffers[buf.index].start,1280*720*2);  
  printf("TK--------->>>resultyuyv is %d\n",resultyuyv);     
  close(fdyuyv);  
  ////  
  close(fd);  
  return 0;  
}  
