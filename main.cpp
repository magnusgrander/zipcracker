#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <zip.h>

const char *prg;
std::vector<std::string>passlist; 

void cleardata(void) 
{
	passlist.clear();
}

std::string trim(const std::string& str) {
	std::string::size_type
		start = str.find_first_not_of(" \t\r\n"),
		end = str.find_last_not_of(" \t\r\n");

	return str.substr(start == std::string::npos ? 0 : start,
			end == std::string::npos ? str.length() - 1 : end - start + 1);
} 

 
static void safe_create_dir(const char *dir)
{
    if (mkdir(dir, 0755) < 0) {
        if (errno != EEXIST) {
            perror(dir);
            exit(1);
        }
    }
}

int unzip(const char *archname) 
{
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100];
    int err;
    int i, len;
    int fd;
    int counter = 0;
    long long sum;
        
    archive = archname;
    if ((za = zip_open(archive, 0, &err)) == NULL) {
        zip_error_to_str(buf, sizeof(buf), err, errno);
        std::cout << "can't open " << archive << std::endl;        
        return 1;
    }
    
    start:    
    std::string password = passlist[counter];
    if(password.length()==0) {
		std::cout << "passwordlist ended" << std::endl;
		exit(0);
	}
        
    int iRetPassword = zip_set_default_password(za, password.c_str());    
    counter++;
         
    for (i = 0; i < zip_get_num_entries(za, 0); i++) {
        if (zip_stat_index(za, i, 0, &sb) == 0) {
            std::cout << "---------------------------" << std::endl;
            len = strlen(sb.name);
            std::cout << "Name:" << sb.name << std::endl;
            std::cout << "Size:" << sb.size << std::endl;
            std::cout << "mtime:" << sb.mtime << std::endl;
            std::cout << "password:" << password << std::endl;
            std::cout << "Length:" << password.size() << std::endl;
            std::cout << "counter:" << counter << std::endl;
                                    
            if (sb.name[len - 1] == '/') {
                safe_create_dir(sb.name);
            } else {
								
                zf = zip_fopen_index(za, i, 0);
                if (!zf) {
                    std::cout << "Data / Password error"  << std::endl;
                    goto start;
                    //exit(100);
                }
 
                fd = open(sb.name, O_RDWR | O_TRUNC | O_CREAT, 0644);
                if (fd < 0) {
                    std::cout << "Read error"  << std::endl;
                    goto start;
                    //exit(101);
                }
 
                sum = 0;
                while (sum != sb.size) {
                    len = zip_fread(zf, buf, 100);
                    if (len < 0) {
                        std::cout << "Filesize error" << std::endl;
                        goto start;
                        //exit(102);
                    }
                    write(fd, buf, len);
                    sum += len;
                }
                close(fd);
                zip_fclose(zf);
            }
        } else {
			std::cout << "file:" <<  __FILE__ << "line:" << __LINE__ << std::endl;            
        }
    }
    std::cout << "Zip file hacked and unpacked."  << std::endl;          
 
    if (zip_close(za) == -1) {
        std::cout << "Can't close archive" << std::endl;
        return 1;
    }
 
    return 0;	
}

static void pwloop(const char *filename)
{
	std::string password;
    std::ifstream file(filename);
    if(!file)
    {
        std::cout<<"Error while opening file";
        exit(1);
    }    
    while(!file.eof())
    {
        file>>password;
        passlist.push_back(trim(password));
    }
    file.close();  
 
}
 
int main(int argc, char *argv[])
{
    prg = argv[0];
    if (argc != 3) {
		std::cout << "Usage: " << prg << " archive" << " dictinary"  << std::endl;        
        return 1;
    }
    else {
		pwloop(argv[2]);
		unzip(argv[1]);		
	}  
    return 0;
}
