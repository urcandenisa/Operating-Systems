#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>
#define BUFF_SIZE 121
#define booleanToString(x) ( x ? "true" : "false")
#define MAX_PATH 1024
typedef struct section{
	char sect_name[20];
	int sect_type;
	int sect_offset;
	int sect_size;
}section_header;
int getPosition(int argc, char **argv, char *string){
	for(int i = 0; i < argc; i++)
	if(strstr(argv[i], string) != NULL){
		return i;
	}
	return -1;
}
const char *filePath(int argc, char **argv){
	int position = getPosition(argc, argv, "path");
	return argv[position] + 5;
}
int getValue(int argc, char **argv){
	int position = getPosition(argc, argv, "size_greater");
	if(position >= 0){
		return atoi(argv[position] + 13);
	}
	return -1;
}
const char *endsWith(int argc, char **argv){
	int position = getPosition(argc, argv, "name_ends_with");
	if(position >= 0){
		return argv[position] + 15;
	}
	return "";
}
bool checkEnd(char *string, const char *match){
	// daca string se termina cu match
	return (strlen(string) >= strlen(match)) && (strcmp(string + (strlen(string) - strlen(match)), match) == 0 );
}
void traverse(const char *filePath, bool recursive, int value, const char *string, int *succes){
	
	DIR *dir = NULL;
	char fullPath[MAX_PATH];
	struct dirent *entry = NULL;
	struct stat statbuff;
	dir = opendir(filePath);
	if(dir == NULL){
		perror("ERROR\ninvalid directory path");
		(*succes) = -1;
		return;
	}
	if((*succes) == 0){
	printf("SUCCESS\n");
	}
	for(;;){
		entry = readdir(dir);
		if(entry == NULL){
			break;
		}
		
		if(!(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)){
			snprintf(fullPath, MAX_PATH, "%s/%s", filePath, entry->d_name);
			
			if(lstat(fullPath, &statbuff) < 0){
				perror("Something went wrong");
				closedir(dir);
				return;
			}

			if(value >= 0){
				if(S_ISREG(statbuff.st_mode)){
					if(statbuff.st_size > value){
							printf("%s\n", fullPath);
					}
				}
				else{
					if(recursive == true){
						if(S_ISDIR(statbuff.st_mode)){
						(*succes) ++;
						traverse(fullPath, recursive, value, string, succes);
						}
					}
				}
			}
			else{
					if(string != NULL){
						if(checkEnd(fullPath, string) == true){
							printf("%s\n", fullPath);
							if(recursive == true){
								if(S_ISDIR(statbuff.st_mode)){
								(*succes) ++;
								traverse(fullPath, recursive, value, string, succes);
								}
							}
						}
					}
					else{
						printf("%s\n", fullPath);
						if(recursive == true){
							if(S_ISDIR(statbuff.st_mode)){
								(*succes) ++;
								traverse(fullPath, recursive, value, string, succes);
						}
					}
				}
			}
		}
	}
	closedir(dir);
}

void parseFile(const char *filePath){
	int fd = -1;
	char magic[4];
	int header_size = 0;
	int version = 0;
	int no_of_sections = 0;
	bool valid = true;
	fd = open(filePath, O_RDONLY); // read only
	if(fd == -1){
		perror("ERROR\nwrong|magic|version|sect_nr|sect_types");
		exit(-1);
	}
	lseek(fd, 0, SEEK_SET);

	read(fd, magic, 4);
	read(fd, &header_size, 2);
	read(fd, &version, 4);
	read(fd, &no_of_sections, 1);

	section_header *section_headers;
	section_headers = (section_header*)calloc(no_of_sections, sizeof(section_header));

	for(int i = 0; i < no_of_sections; i++){
		read(fd, section_headers[i].sect_name, 12);
		read(fd, &section_headers[i].sect_type, 2);
		read(fd, &section_headers[i].sect_offset, 4);
		read(fd, &section_headers[i].sect_size, 4);
	}

	if(strcmp(magic, "51SY") != 0){
		valid = false;
		printf("ERROR\nwrong magic");
		free(section_headers);
		close(fd);
		return;
	}
	if(!( version >= 111 && version <= 169 ))
	{
		valid = false;
		printf("ERROR\nwrong version");
		free(section_headers);
		close(fd);
		return;
	}
	if(!( no_of_sections >= 7 && no_of_sections <=10)){
		valid = false;
		printf("ERROR\nwrong sect_nr");
		free(section_headers);
		close(fd);
		return;
	}

	for(int i = 0; i < no_of_sections; i++){
		if((section_headers[i].sect_type != 77 && section_headers[i].sect_type != 16 && section_headers[i].sect_type != 49 && section_headers[i].sect_type != 45))
		valid = false;
	}

	if(valid == false){
		printf("ERROR\nwrong sect_types");
		free(section_headers);
		close(fd);
		return;
	}
	
	printf("SUCCESS\nversion=%d\nnr_sections=%d\n", version, no_of_sections);
	for(int i = 0; i < no_of_sections; i++){
		printf("section%d: %s %d %d\n", i+1, section_headers[i].sect_name, section_headers[i].sect_type, section_headers[i].sect_size);
	}

	free(section_headers);
	close(fd);
}

char *strrev(char *string){
		char c, *front, *back;
	    if(!string || !*string)
	        return string;
	    for(front=string,back=string+strlen(string)-1;front < back;front++,back--){
	        c=*front;*front=*back;*back=c;
	    }
	    return string;
}

void extractLine(const char *filePath, int section, int line){
	int fd = -1;
	char magic[4];
	int header_size = 0;
	int version = 0;
	int no_of_sections = 0;
	bool valid = true;
	int offset = -1;
	int size = -1;
	int i = 0;
	int count = 0;
	char *lin;
	fd = open(filePath, O_RDONLY); // read only
	if(fd == -1){
		perror("ERROR\ninvalid file");
		exit(-1);
	}
	lseek(fd, 0, SEEK_SET);

	read(fd, magic, 4);
	read(fd, &header_size, 2);
	read(fd, &version, 4);
	read(fd, &no_of_sections, 1);

	section_header *section_headers;
	section_headers = (section_header*)calloc(no_of_sections, sizeof(section_header));

	for( i = 0; i < no_of_sections; i++){
		read(fd, section_headers[i].sect_name, 12);
		read(fd, &section_headers[i].sect_type, 2);
		read(fd, &section_headers[i].sect_offset, 4);
		read(fd, &section_headers[i].sect_size, 4);
	}

	if(strcmp(magic, "51SY") != 0){
		valid = false;
		printf("ERROR\nwrong magic");
		free(section_headers);
		close(fd);
		return;
	}
	if(!( version >= 111 && version <= 169 ))
	{
		valid = false;
		printf("ERROR\nwrong version");
		free(section_headers);
		close(fd);
		return;
	}
	if(!( no_of_sections >= 7 && no_of_sections <=10)){
		valid = false;
		printf("ERROR\nwrong sect_nr");
		free(section_headers);
		close(fd);
		return;
	}

	for(i = 0; i < no_of_sections; i++){
		if((section_headers[i].sect_type != 77 && section_headers[i].sect_type != 16 && section_headers[i].sect_type != 49 && section_headers[i].sect_type != 45))
		valid = false;
	}

	if(valid == false){
		printf("ERROR\nwrong sect_types");
		free(section_headers);
		close(fd);
		return;
	}

	offset = section_headers[section - 1].sect_offset;
    size = section_headers[section-1].sect_size;
    if(offset == -1){
    	printf("ERROR\ninvalid section");
    	return;
    }
	lseek(fd, offset, SEEK_SET);
	lin = (char*)calloc(size, sizeof(char));
	count = 0;
	i = 0;
	lseek(fd, offset, SEEK_SET);
	lseek(fd, size, SEEK_CUR);
	lseek(fd, -1, SEEK_CUR);
	while(read(fd, &lin[i], 1) != 0){
		i = 0;
		lseek(fd, -1, SEEK_CUR);
		while(lin[i]!='\n'){
			i++;
			lseek(fd, -1, SEEK_CUR);

			if(read(fd, &lin[i], 1) != 1){
				break;
			}
			lseek(fd, -1, SEEK_CUR);
		}
		lin[i] = 0;
	    lin = strrev(lin);
		count ++;
		if(count == line){
			break;
		}
		i = 0;
		lseek(fd, -2, SEEK_CUR);
		free(lin);
		lin =  (char*)calloc(size, sizeof(char));
	}
	if(strcmp(lin, "") == 0){
		printf("ERROR\ninvalid line");
		return;
	}
	else
		if(valid == true){
		printf("SUCCESS\n%s\n", lin);
		}
	free(lin);

	free(section_headers);
	close(fd);
}
void readAll(const char*filePath, char *magic, int *header_size, int *version, int *no_of_sections, section_header *section_headers){
	int fd = open(filePath, O_RDONLY); // read only
	int i;
	if(fd == -1){
		perror("ERROR\ninvalid file");
		exit(-1);
	}
	lseek(fd, 0, SEEK_SET);
	read(fd, magic, 4);
	read(fd, header_size, 2);
	read(fd, version, 4);
	read(fd, no_of_sections, 1);

	for( i = 0; i < *no_of_sections; i++){
		read(fd, section_headers[i].sect_name, 12);
		read(fd, &section_headers[i].sect_type, 2);
		read(fd, &section_headers[i].sect_offset, 4);
		read(fd, &section_headers[i].sect_size, 4);
	}
	close(fd);
}

int numberOfSections(const char *filePath){
	int fd = -1;
	int number = 0;
	fd = open(filePath, O_RDONLY);
	if(fd == -1){
		perror("ERROR\ninvalid file");
		exit(-1);
	}
	lseek(fd, 10, SEEK_SET);
	read(fd, &number, 1);
	close(fd);
	return number;
}

void findall(const char*filePath, int *succes){
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	struct stat statbuff;
	char path[MAX_PATH];
	
	dir = opendir(filePath);
	if(dir == NULL){
		perror("ERROR\ninvalid directory path");
		(*succes)++;
		return;
	}
	if(*succes == 0){
		printf("SUCCESS\n");
	}
	for(;;){
		entry = readdir(dir);
		if(entry == NULL){
			break;
		}
		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
			snprintf(path, MAX_PATH, "%s/%s", filePath, entry->d_name);
			if(lstat(path, &statbuff) < 0){
				perror("Something went wrong");
				closedir(dir);
				return;
			}
			
			if(S_ISREG(statbuff.st_mode)){
			char magic[5];
			magic[4]= 0;
  		    int header_size = 0;
            int version = 0;
            int no_of_sections = 0;
            bool valid = true;
			int number = numberOfSections(path);
        	section_header *section_headers = (section_header*)calloc(number, sizeof(section_header));
        	readAll(path, magic, &header_size, &version, &no_of_sections, section_headers);
        	if(strcmp(magic, "51SY") != 0)
        		valid = false;
        	if((!( version >= 111 && version <= 169 )) || (!( no_of_sections >= 7 && no_of_sections <=10)))
					valid = false;
			if(valid == true)
			for(int i = 0; i < no_of_sections; i++){
				if(section_headers[i].sect_size > 1000){
				valid = false;
				break;
			}
			}
        	if(valid == true){
        		printf("%s\n", path);
        	}
        	free(section_headers);
        	}
			else if(S_ISDIR(statbuff.st_mode)){
				(*succes)++;
				findall(path, succes);
			}

		}
	}
	closedir(dir);
}

int main(int argc, char **argv){
	bool recursive = false;
	int value = -1;
	const char *string = NULL;
	const char *path = NULL;
	int succes = 0;
	int position = -1;
	int section = 0;
	int line = 0;
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("72987\n");
        }
        if(strcmp(argv[1], "list") == 0){
        	path = filePath(argc, argv);
        	if(getPosition(argc, argv, "recursive") > 0){
        		recursive = true;
        	}
        	value = getValue(argc, argv);
        	string = endsWith(argc, argv);
        	traverse(path, recursive, value, string, &succes);
        }
        position = getPosition(argc, argv, "parse");
        if(position != -1){
        	path = filePath(argc, argv);
        	parseFile(path);
        }
        if(strcmp(argv[1], "extract") == 0){
        	path = filePath(argc, argv);
        	sscanf(argv[3]+8, "%d", &section);
        	sscanf(argv[4]+5, "%d", &line);
        	extractLine(path, section, line);
        }
        if(strcmp(argv[1],"findall") == 0){
        	path = filePath(argc, argv);
        	int succes = 0;
        	findall(path, &succes);
      
        }
    }

    return 0;
}