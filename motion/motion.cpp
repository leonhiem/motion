#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <syslog.h>
#include <boost/asio.hpp>

#include "base64.h"


using namespace std;
using boost::asio::ip::tcp;

tcp::iostream sock;
std::string myname="noname";

int connect_ipcamera(const char *host, const char *port)
{
  size_t base64_len;
  const char *username="admin:";
  build_decoding_table();
  try {

    sock.expires_from_now(boost::posix_time::seconds(60));
    sock.connect(host, port);
    if (!sock) {
      syslog(LOG_ERR,"Connect ipcamera: %s\n",sock.error().message().c_str());
      return -1;
    }
    sock << "GET /videostream.cgi?resolution=32 HTTP/1.1\r\n";
    if(myname == "smederij") {
        sock << "Authorization: Digest username=\"admin\", realm=\"ipcamera_00626E568F7C\", nonce=\"62de33d8045a6bab01259739190ec3ce\", uri=\"/videostream.cgi\", cnonce=\"ICAgICAgICAgICAgICAgICAgICAgICAgIDE3MTA1NTQ=\", nc=00000001, qop=auth, response=\"5e579139fb0ed66dfadf0afe44b9ae6b\", algorithm=\"MD5\"\r\n";
        //sock << "User-Agent: curl/7.31.0\r\n";
    } else {
        sock << "Authorization: basic " << base64_encode(username,strlen(username),&base64_len) << "\r\n";
    }
    sock << "Host: " << host << "\r\n";
    //sock << "Accept: */*\r\n";
    sock << "\r\n" << std::flush;

    //cout << sock.rdbuf();
  } catch (std::exception& e) {
    syslog(LOG_ERR,"[%s] Connect ipcamera: %s\n",myname.c_str(),e.what());
    return -1;
  }
  return 0;
}

int read_header(void)
{
    char line[150];

    sock.getline(line, sizeof(line)); // read: ^M
    strcat(line,"\n");
    if(!sock) throw("socket read error");

    sock.getline(line, sizeof(line)); // read: Content-Type: image/jpeg^M
    strcat(line,"\n");
    if(!sock) throw("socket read error");

    sock.getline(line, sizeof(line)); // read: Content-Length: 44368^M
    strcat(line,"\n");
    if(!sock) throw("socket read error");

    string prefx;
    int cont_len;
    stringstream ss(line);
    ss >> prefx;
    ss >> cont_len;

    //cout << "cont_len=" << cont_len << endl;
    sock.getline(line, sizeof(line)); // read: ^M
    if(!sock) throw("socket read error");
    strcat(line,"\n");
    return cont_len;
}


int takeout1jpeg(const char *grep_str, ofstream& f_A)
{
    int content_len=0;

    char *hdrbuffer = new char[strlen(grep_str)+1];
    try {
        while(1) {
            sock.read(hdrbuffer, 1);
            if(sock.gcount() != 1) throw("empty read [1]");

            if(hdrbuffer[0]==grep_str[0]) {
                sock.read(&hdrbuffer[1], strlen(grep_str)-1);
                if(sock.gcount() != strlen(grep_str)-1) throw("empty read [2]");

                if(strncmp(hdrbuffer,grep_str,strlen(grep_str)) == 0) {

                    content_len=read_header();
                    if(content_len==0) throw("empty header");

                    char *cinbuffer = new char[content_len+1];
                    sock.read(cinbuffer, content_len);
                    if(sock.gcount() != content_len) {
                        delete[] cinbuffer;
                        throw("empty read [3]");
                    }

                    f_A.write(cinbuffer, content_len); 
                    delete[] cinbuffer;

                    break;
                }
            }
        }
    } catch(std::exception& e) {
        //cerr << "Catch: " << e.what() << endl;
        syslog(LOG_ERR,"[%s] Catch: [%s]\n",myname.c_str(),e.what());
        content_len=0;
    } catch(const char *str) {
        //cerr << "Catch: " << str << endl;
        syslog(LOG_ERR,"[%s] Catch: [%s]\n",myname.c_str(),str);
        content_len=0;
    } catch(...) {
        //cerr << "Catch: ..." << endl;
        syslog(LOG_ERR,"[%s] Catch undefined error\n",myname.c_str());
        content_len=0;
    }
    delete[] hdrbuffer;
    return content_len;
}

string exec_system_cmd(const char* cmd) throw(const char*)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw("Error popen()");
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
    }
    pclose(pipe);
    return result;
}

#define DIR_DATA         "/home/hiemstra/public_html/camera"
#define DIR_RAMDISK      "/mnt/ramdisk"
string get_ram_fname(string prefix,string name) {
    return(string(DIR_RAMDISK)+"/"+name+"-"+prefix+".jpg");
}

void add_polygons(const char *fname, const char *pfname, const char *prfname, 
                  const string& polygon)
{
    ostringstream cmd;
    if(polygon=="") {
        cmd << "cp " << fname << " " << prfname << " 1>/dev/null 2>&1";
    } else {
        cmd << "convert " << fname << " -fill black -draw \"polygon " << polygon << "\" "
            << prfname << " 1>/dev/null 2>&1";
    }
    system(cmd.str().c_str());
}

void crop(const char *fname, const char *pfname, const char *prfname, 
          const string& sizes)
{
    ostringstream cmd;
    if(sizes=="") {
        cmd << "cp " << fname << " " << prfname << " 1>/dev/null 2>&1";
    } else {
        cmd << "convert " << fname << " -crop " << sizes << " "
            << prfname << " 1>/dev/null 2>&1";
    }
    system(cmd.str().c_str());
}

main(int argc, char* argv[])
{
    int tm_day_old=-1;
    struct timeval t;
    struct tm *td = new(struct tm);
    ofstream fileA, fileB;
    const char *grep_str="--ipcamera";
    int content_len=0;

  try {
    if(argc < 6) throw("Usage: "+string(argv[0])+" myname hostname port fuzzfactor threshold disable_extra").c_str();

    myname=string(argv[1]);
    int fuzzfactor=atoi(argv[4]);
    int threshold=atoi(argv[5]);
    int disable_extra=0;

    if(argc > 6) disable_extra=atoi(argv[6]);

    /*  Open syslog connection */
    openlog(basename(argv[0]),0,LOG_LOCAL0);
    syslog(LOG_INFO,"Starting motion detector [%s]\n",myname.c_str());
    if(disable_extra) syslog(LOG_INFO,"disabling extra\n");

    if(connect_ipcamera(argv[2],argv[3]) != 0) throw("Cannot connect to ipcamera");

    string horizon_str="";
    string polygon_str="";
    string crop_str="";

    if(myname == "smederij") {
        horizon_str="480,90 360,480";
        polygon_str="0,0 0,280 80,280 120,90 "+horizon_str+" 640,480 640,0";
        crop_str="320x380+100+100";
    } else if(myname == "kalverstal") {
        horizon_str="0,105 400,105";
        polygon_str="0,0 "+horizon_str+" 470,240 640,240 640,0";
        crop_str="640x230+0+250";
    } else if(myname == "robot1") {
        horizon_str="0,160 640,160";
        polygon_str=horizon_str+" 640,480 0,480   0,110 640,110 640,0 0,0"; 
        crop_str="640x50+0+115";
    } else if(myname == "robot2") {
        horizon_str="0,120 640,120";
        polygon_str=horizon_str+" 640,480 0,480";
        crop_str="640x50+0+90";
    }


    int toggle=0;
    int first=1;
    int diff=0;
    int toomanyfiles=0;
    time_t toomanyfiles_oldtime=0;
    while(1) {

      // watchdog reset
      sock.expires_from_now(boost::posix_time::seconds(30));

      string fname_A    = get_ram_fname("a",myname);
      string fname_A_P  = get_ram_fname("pa",myname);
      string fname_A_PR = get_ram_fname("pra",myname);

      if(first) {
          fileA.open(fname_A.c_str(),ios::out | ios::binary);
          if(!fileA) throw(("1 cannot open [a] file: " + string(strerror(errno))).c_str());
          if((content_len=takeout1jpeg(grep_str,fileA))==0) {
              syslog(LOG_ERR,"jpeg not found [A1]!");
              fileA.close();
              break;
          }
          fileA.close();
          //add_polygons(fname_A.c_str(),fname_A_P.c_str(),fname_A_PR.c_str(),polygon_str);
          crop(fname_A.c_str(),fname_A_P.c_str(),fname_A_PR.c_str(),crop_str);
          first=0;
      }

      string fname_B    = get_ram_fname("b",myname);
      string fname_B_P  = get_ram_fname("pb",myname);
      string fname_B_PR = get_ram_fname("prb",myname);
      string fname_diff = get_ram_fname("diff",myname);

      usleep(500000);
      if(toggle==0) {
          fileB.open(fname_B.c_str(),ios::out | ios::binary);
          if(!fileB) throw(("2 cannot open [b] file: " + string(strerror(errno))).c_str());
          if((content_len=takeout1jpeg(grep_str,fileB))==0) {
              syslog(LOG_ERR,"jpeg not found [B]!");
              fileB.close();
              break;
          } 
          fileB.close();
          //add_polygons(fname_B.c_str(),fname_B_P.c_str(),fname_B_PR.c_str(),polygon_str);
          crop(fname_B.c_str(),fname_B_P.c_str(),fname_B_PR.c_str(),crop_str);
          toggle=1;
      } else {
          fileA.open(fname_A.c_str(),ios::out | ios::binary);
          if(!fileA) throw(("3 cannot open [a] file: " + string(strerror(errno))).c_str());
          if((content_len=takeout1jpeg(grep_str,fileA))==0) {
              syslog(LOG_ERR,"jpeg not found [A]!");
              fileA.close();
              break;
          }
          fileA.close();
          //add_polygons(fname_A.c_str(),fname_A_P.c_str(),fname_A_PR.c_str(),polygon_str);
          crop(fname_A.c_str(),fname_A_P.c_str(),fname_A_PR.c_str(),crop_str);
          toggle=0;
      }




      ostringstream cmp_cmd;
      cmp_cmd << "compare -verbose -metric AE -fuzz " << fuzzfactor << "\% " 
              //<< "-compose Src -highlight-color Red -lowlight-color Transparent "
              << fname_A_PR << " " << fname_B_PR << " " << fname_diff << " 2>&1 | grep all";
              //<< fname_A_PR << " " << fname_B_PR << " null: 2>&1 | grep all";
      string response = exec_system_cmd(cmp_cmd.str().c_str());
      stringstream ss(response);
      string all_str;
      ss >> all_str;
      ss >> diff;

   
      if(diff > threshold) {
        string toomanyfiles_str="";
        char label[200];
        char filenamelabel[200];
        gettimeofday(&t,NULL);
        td = localtime(&t.tv_sec);
        sprintf(label,"%04d-%02d-%02d %02d:%02d:%02d",td->tm_year+1900,td->tm_mon+1,td->tm_mday,
                                                      td->tm_hour,td->tm_min,td->tm_sec);
        sprintf(filenamelabel,"%02d%02d%02d-%d",td->tm_hour,td->tm_min,td->tm_sec,diff);

        // td->tm_wday: 0-6 (0=sunday)
                const char* weekday[] = {
                        "zondag",   // 0
                        "maandag",  // 1
                        "dinsdag",  // 2
                        "woensdag", // 3
                        "donderdag",// 4
                        "vrijdag",   
                        "zaterdag" 
                };
                int day_part=(td->tm_hour >= 12);
                const char* ampm[] = {
                        "am",   
                        "pm" 
                };

        if(tm_day_old == -1) {
            tm_day_old=td->tm_wday; // first time

            ostringstream delete_cmd;
            // delete files older than 3 days anyways:
            delete_cmd << "find " << DIR_DATA << "/" << myname << "/" << weekday[td->tm_wday] << "/" << ampm[0] 
                       << "/ -mtime +3 -exec rm -f {} \\;";
            system(delete_cmd.str().c_str());

            delete_cmd.clear(); delete_cmd.str("");
            delete_cmd << "find " << DIR_DATA << "/" << myname << "/" << weekday[td->tm_wday] << "/" << ampm[1] 
                       << "/ -mtime +3 -exec rm -f {} \\;";
            system(delete_cmd.str().c_str());
        }

	if((t.tv_sec-toomanyfiles_oldtime) < 5) {
	    toomanyfiles++;
	} else {
	    toomanyfiles=0;
	}
	toomanyfiles_oldtime=t.tv_sec;

	if(toomanyfiles > 3) {
            toomanyfiles_str="extra";
            if(disable_extra) {
                syslog(LOG_INFO,"[%s] ignore\n",myname.c_str());
                continue;
            }
	} else {
            toomanyfiles_str="";
	}
        syslog(LOG_INFO,"[%s] diff=%d time=%s (%d %s)\n",myname.c_str(),diff,label,toomanyfiles,toomanyfiles_str.c_str());


        if(td->tm_wday != tm_day_old) { // empty new directory
            ostringstream delete_cmd;
            delete_cmd << "rm " << DIR_DATA << "/" << myname << "/" << weekday[td->tm_wday] << "/*/* ; ";
            delete_cmd << "rm " << DIR_DATA << "/" << myname << "/" << weekday[td->tm_wday] << "/*/*/*";
            system(delete_cmd.str().c_str());
            toomanyfiles_str="";
	    toomanyfiles=0;
        }
        tm_day_old = td->tm_wday;

        // if toggle==1: B is new
        // if toggle==0: A is new

        ostringstream label_cmd;
        label_cmd << "convert " << (toggle==0?fname_A:fname_B) 
                  //<< " -fill red -draw \"line "
                  //<< horizon_str << "\""
                  << " -pointsize 25 -draw \"gravity south "
                  << "fill black text 0,12 '" << label << "' "
                  << "fill white text 1,11 '" << label << "' \" "
                  << DIR_DATA << "/" << myname << "/" 
                  << weekday[td->tm_wday] << "/" << ampm[day_part] << "/" << toomanyfiles_str << "/" << myname << "-" << filenamelabel 
                  << ".jpg 1>/dev/null 2>&1 ; "

                  //<< " cp " << fname_diff << " " 
                  //<< DIR_DATA << "/" << myname << "/"
                  //<< weekday[td->tm_wday] << "/" << ampm[day_part] << "/" << toomanyfiles_str << "/" << myname << "-" << filenamelabel
                  //<< "shadow.png 1>/dev/null 2>&1 ; "
                  ;
        system(label_cmd.str().c_str());
      }
    }
  } catch(const char *str) {
    //cerr << "Catch: " << str << endl;
    syslog(LOG_ERR,"[%s] Error catched: [%s]\n",myname.c_str(),str);
  }
  closelog(); // close syslog connection
}

