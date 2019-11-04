#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <math.h>
typedef struct section{
	//int sect_type;
	int sect_offset;
	int sect_size;
}section_header;

#define response_pipe "RESP_PIPE_72987"
#define request_pipe "REQ_PIPE_72987"
#define my_number 72987
#define SHM_KEY 17500

int main(void){

	int req_pipe = -1, resp_pipe = -1;
	//int fd = -1;
	unsigned char length_message = 0;
	char message[255];
	
	int shm_id = -1;
	char *shared = NULL;

	//int fd = -1, size = 0;
	int size = 0;
	char *data = NULL;
			
	char file_path[255] = "";
	unsigned char length_file_path;

	char connect[] = "CONNECT";
	unsigned char length = sizeof(connect) - 1;

	//create fifo -- pipe with name: resp
	if(mkfifo(response_pipe, 0644) != 0){
		perror("Could not create pipe");
		return -1;
	}

	//open fifo -- pipe with name: req
	if((req_pipe = open(request_pipe, O_RDONLY)) == -1){
		perror("Could not open pipe: req");
		return -3;
	}

	//open fifo --pipe with name: resp
	if((resp_pipe = open(response_pipe, O_WRONLY)) == -1){
		perror("Could not open pipe: resp");
		return -2;
	}

	//connect with the pipe
	write(resp_pipe, &length, sizeof(length));
	write(resp_pipe, connect, length);

	//printf("SUCCESS\n");

	for(;;){
		//read the message
		
		read(req_pipe, &length_message, sizeof(length_message));
		
		read(req_pipe, &message, length_message);
		
		message[length_message] = 0;
		
		if(strncmp(message, "PING", 4) == 0){
			//PING

			unsigned int number = my_number;

			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, "PING", length_message);
			
			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, "PONG", length_message);

			write(resp_pipe, &number, sizeof(number));

			//printf("SUCCESS");

		}else if(strncmp(message, "CREATE_SHM", 9) == 0){
			//CREATE_SHM

			unsigned int memory_dimension = 0;

			read(req_pipe, &memory_dimension, sizeof(memory_dimension));

			shm_id = shmget(SHM_KEY, memory_dimension, IPC_CREAT | 0644);

			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, "CREATE_SHM", length_message);

			if(shm_id < 0){

				unsigned char error_message_dimension = 5;
				
				write(resp_pipe, &error_message_dimension, sizeof(error_message_dimension));
				write(resp_pipe, "ERROR", error_message_dimension);
				//printf("ERROR\n");

			}else{

				unsigned char success_message_dimension = 7;

				write(resp_pipe, &success_message_dimension, sizeof(success_message_dimension));
				write(resp_pipe, "SUCCESS", success_message_dimension);
				//printf("SUCCESS\n");
			}

		}else if(strncmp(message, "WRITE_TO_SHM", 12) == 0){
			//WRITE_TO_SHM

			unsigned int offset;
			unsigned int value;

			read(req_pipe, &offset, sizeof(offset));
			read(req_pipe, &value, sizeof(value));

			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, message, length_message);

			if(offset >= 0 && offset + 4 <= 4970339){

				shared = (char*)shmat(shm_id, 0, 0);
				
				
				*((int*) (shared +offset)) = value;
				
				//shmdt(shared);
			//	shared = NULL;
			//	shmctl(shm_id, IPC_RMID, 0);
				
				unsigned char success_message_dimension = 7;

				write(resp_pipe, &success_message_dimension, sizeof(success_message_dimension));
				write(resp_pipe, "SUCCESS", success_message_dimension);
			//	printf("SUCCESS\n");

			}else{

				unsigned char error_message_dimension = 5;
				
				write(resp_pipe, &error_message_dimension, sizeof(error_message_dimension));
				write(resp_pipe, "ERROR", error_message_dimension);
				//shmdt(shared);
				//shared = NULL;
				//shmctl(shm_id, IPC_RMID, 0);
				//printf("ERROR\n");

			}
		}else if(strncmp(message, "MAP_FILE", 8) == 0){
			//MAP FILE

			int fd = -1;

			read(req_pipe, &length_file_path, sizeof(length_file_path));
			read(req_pipe, file_path, length_file_path);
			file_path[length_file_path] = 0;
			fd = open(file_path, O_RDONLY);
			if (fd == -1){
				perror("error fd\n");
			}

			size = lseek(fd, 0, SEEK_END);
			lseek(fd, 0, SEEK_SET);
			
			data = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
				
			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, message, length_message);
		

			if(data == ((void*) - 1)){
				
				unsigned char error_message_dimension = 5;
				
				write(resp_pipe, &error_message_dimension, sizeof(error_message_dimension));
				write(resp_pipe, "ERROR", error_message_dimension);
				munmap(data, size);
   				close(fd);
			}else{

			unsigned char success_message_dimension = 7;

			write(resp_pipe, &success_message_dimension, sizeof(success_message_dimension));
			write(resp_pipe, "SUCCESS", success_message_dimension);
			//munmap(data, size);
			}
			//printf("SUCCESS\n");
		}else if(strncmp(message, "READ_FROM_FILE_OFFSET", 21) == 0){

			unsigned int offset_1;
			unsigned int no_of_bytes;

			//data = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

			read(req_pipe, &offset_1, sizeof(offset_1));
			read(req_pipe, &no_of_bytes, sizeof(no_of_bytes));

			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, message, length_message);

			if(data == (void*) - 1 || (offset_1 + no_of_bytes) > size){

			unsigned char error_message_dimension = 5;
			write(resp_pipe, &error_message_dimension, sizeof(error_message_dimension));
			write(resp_pipe, "ERROR", error_message_dimension);
			//munmap(data, size);
			}else{

			shared = (char*)shmat(shm_id, 0, 0);
			
			strncpy(shared, data + offset_1, no_of_bytes);
		
			unsigned char success_message_dimension = 7;

			write(resp_pipe, &success_message_dimension, sizeof(success_message_dimension));
			write(resp_pipe, "SUCCESS", success_message_dimension);

			}

		}else if(strcmp(message, "READ_FROM_FILE_SECTION") == 0){
			
			int section_no;
			unsigned int offset_2;
			unsigned int no_of_bytes_2;

			//data = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

			int no_of_sections = data[10];
			
			read(req_pipe, &section_no, sizeof(section_no));
			read(req_pipe, &offset_2, sizeof(offset_2));
			read(req_pipe, &no_of_bytes_2, sizeof(no_of_bytes_2));
				
			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, message, length_message);
			
			if(section_no < 1 || section_no > no_of_sections){

				unsigned char error_message_dimension = 5;
				write(resp_pipe, &error_message_dimension, sizeof(error_message_dimension));
				write(resp_pipe, "ERROR", error_message_dimension);
			//	munmap(data, size);
			}else{

				/*int pos = 25;

				section_header *section_headers;
				section_headers = (section_header*)calloc(no_of_sections, sizeof(section_header));

				for(int i = 0; i < no_of_sections; i++){
				section_headers[i].sect_offset = *((int*)(data + pos));
				section_headers[i].sect_size = *((int*)(data + pos + 4));
				pos += 22;

				}

				int offset_3 = section_headers[section_no - 1].sect_offset;
				int size_3 = section_headers[section_no - 1].sect_size;*/

				shared = (char*)shmat(shm_id, 0, 0);
				
				int pos = 0;
				memcpy(&pos, data + 25 + (section_no - 1)*22, 4);
				strncpy(shared, data + offset_2 + pos , no_of_bytes_2);

				unsigned char success_message_dimension = 7;

				write(resp_pipe, &success_message_dimension, sizeof(success_message_dimension));
				write(resp_pipe, "SUCCESS", success_message_dimension);
			}
		}
		else if(strcmp(message, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0){

			unsigned int logical_offset;
			unsigned int no_of_bytes_3;
			int sect_offset = 0;
			int no_of_sections = data[10];
			int size_section = 0;
			int off_aux = 0, off_logic = 0;
		
			read(req_pipe, &logical_offset, sizeof(logical_offset));
			read(req_pipe, &no_of_bytes_3, sizeof(no_of_bytes_3));

			//printf("%s\n", message);
			write(resp_pipe, &length_message, sizeof(length_message));
			write(resp_pipe, message, length_message);

			int pos = 25;

			shared = (char*)shmat(shm_id, 0, 0);
			
			for(int i = 0; i < no_of_sections; i++){
				size_section = *(int*)(data + pos + 4);
				sect_offset = *(int*)(data + pos);

				off_logic = off_aux;
				off_aux += (size_section/1024 + 1)*1024;
				pos+=22;

				if(off_aux > logical_offset) break;
			}
			
			strncpy(shared, data + sect_offset + logical_offset - off_logic, no_of_bytes_3);

			if(data == (void*) - 1 || logical_offset - sect_offset + no_of_bytes_3 > size){
				unsigned char error_message_dimension = 5;
				
				write(resp_pipe, &error_message_dimension, sizeof(error_message_dimension));
				write(resp_pipe, "ERROR", error_message_dimension);
			} else{

				unsigned char success_message_dimension = 7;

				write(resp_pipe, &success_message_dimension, sizeof(success_message_dimension));
				write(resp_pipe, "SUCCESS", success_message_dimension);
			}
		}
		else if(strncmp(message, "EXIT", 4) == 0){
			break;
		}
	}

	//close pipes
	close(req_pipe);
	close(resp_pipe);

	unlink(response_pipe);

	return 0;
}