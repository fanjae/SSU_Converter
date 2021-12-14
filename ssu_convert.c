#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include "ssu_token.h"
#include "ssu_extern.h"

#define SECOND_TO_MICRO 10000000

char token_list[1000][500];
char mainname[30];
char not_mainname[30];
char matching_function_list[50][500];
int option_state[6]={0}; // -j, -c, -p, -f, -l, -r Option state;

int function_using[50];
int option_read(int argc, char* argv[]); // Option을 읽어오는 함수이다.
void print_matching_function(void); // JAVA와 C언어 매칭되는 함수를 출력한다.
void print_language_code(char* Filename); // Code를 출력하는 함수. Filename을 인자로 받는다. 리턴 값은 없다.
void print_file_size(char* Filename); // 파일의 크기를 출력하는 함수이다. 리턴 값은 없다.
void print_line(char* Filename); // 파일의 라인 수를 출력하는 함수이다. 리턴 값은 없다.
int convert(); // java -> C로 변환하는 함수. 실패시 -1을 출력.
void make_head(); // header를 만든다.

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t) // MICRO 단위의 시간 측정 함수
{
	end_t -> tv_sec -= begin_t -> tv_sec;
	if(end_t -> tv_usec < begin_t -> tv_usec) {
		end_t -> tv_usec--;
		end_t -> tv_usec += SECOND_TO_MICRO;
	}
	
	end_t -> tv_usec -= begin_t -> tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n",end_t -> tv_sec ,end_t -> tv_usec);
}

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t,NULL); // 시간 측정 시작

	if(argc < 2) { // argument로 실행파일만 입력받는 경우 오류 처리.
		fprintf(stderr, "Usage a File : %s\n",argv[0]);
		exit(1);
	}

	if((access(argv[1], F_OK)) < 0) { // .java 파일이 존재하지 않으면 오류 처리.
		fprintf(stderr, "%s doesn't exist.\n",argv[1]);
		exit(1);
	}

	if((option_read(argc,argv)) < 0) { // Option을 읽어와서 존재하지 않은 옵션이 있을 경우 오류 처리.
		fprintf(stderr, "Some option it's not exist.\n");
		exit(1);
	}

	
	make_head(); // header 파일 생성
	if(convert(argv[1]) == -1) {
		fprintf(stderr, "Convert Error\n");
		exit(1);
	}
	

	if(option_state[0] > 0) // -j 옵션 실행
		print_language_code(argv[1]); 
	if(option_state[1] > 0) { // -c 옵션 실행
		print_language_code(mainname);	
		if(strlen(not_mainname) != 0) // 파일이 2개 이상으로 변환된 경우
			print_language_code(not_mainname);
	}
	if(option_state[2] > 0) { // -p 옵션 실행
		print_matching_function();
	}
	if(option_state[3] > 0) { // -f 옵션 실행
		print_file_size(argv[1]);
	}
	if(option_state[4] > 0) { // -l 옵션 실행
		print_line(argv[1]);
	}
	
	gettimeofday(&end_t, NULL); // 시간 측정 종료
	ssu_runtime(&begin_t,&end_t); // 시간 측정 출력
}

int option_read(int argc, char* argv[]) // option check. 옵션의 개수를 리턴함. (단, 존재하지 않는 옵션 존재시 -1을 리턴.)  
{
	int count = 0;
	int n_count = 0;
	for(int i = 2; i < argc; i++) {
		if(strcmp(argv[i],"-j") == 0) { // -j : 변환할 Java language program Code Print.
			option_state[0]++;
			count++;
		}
		else if(strcmp(argv[i],"-c") == 0) { // -c : 변환된 c language program Code Print.
			option_state[1]++;
			count++;
		}
		else if(strcmp(argv[i],"-p") == 0) { // -p : Java -> C language program 대응되는 Function Print.
			option_state[2]++;
			count++;
		}
		else if(strcmp(argv[i],"-f") == 0) { // -f : Java language program file size Print.
			option_state[3]++;
			count++;
		}
		else if(strcmp(argv[i],"-l") == 0) { // -l : Java language program file line & C language program file line
			option_state[4]++;
			count++;
		}
		else if(strcmp(argv[i],"-r") == 0) { // -r : child process Java program file convert to C program file. (한 줄씩 변환과정 보임.) 
			option_state[5]++;
			count++;
		}
		else {
			n_count++;
			break;
		}
	}
	if(n_count == 0)
		return count;
	else
		return -1;
}
int convert(char *Filename) 
{
	FILE *fp;
	char buf[100];
	
	if((fp = fopen(Filename,"r")) == NULL) {
		fprintf(stderr,"File Open Error for : %s in convert function()", Filename);
		exit(1);
	}


	while (fgets(buf, sizeof(buf),fp) != NULL) { 
		tokenizer(buf);
	}
	token_to_convert();		
	name_set();
}
void print_language_code(char *Filename) // -j 옵션에 대한 처리
{
	int fd;
	int line_cnt=0;
	char buf;

	if((fd = open(Filename,O_RDONLY,0644)) < 0) { // Filename으로 파일을 읽기 모드로 읽어온다.
		fprintf(stderr,"File Open Error for : %s in print_java_code function()",Filename);
		exit(1);
	}

	while(read(fd,&buf,1) > 0) { // 엔터만큼 읽어서 line 개수를 센다.
		if(buf == '\n')
			line_cnt++;
	}		
	lseek(fd,0,SEEK_SET); // 파일을 처음으로 이동.

	for(int i = 0; i < line_cnt; i++) { // 엔터 개수 만큼 반복한다.
		printf("%d ", i+1);
		while(1) {
			read(fd, &buf, 1); // 1글자씩 읽어온다.
			printf("%c",buf); // buf값 출력

			if(buf == '\n') // 엔터면 break
				break;
		}
	}
	close(fd);
}
void print_matching_function(void)
{
	for(int i = 0; i < 6; i++) {
		if(function_using[i] == 1) {
			printf("%d %s\n",i+1,matching_function_list[i]); // matching 되는 함수를 출력한다. 
		}
	}
}
void print_file_size(char *filename)
{
	int fd,size;
	fd = open(filename,O_RDONLY,0644); // java 파일을 읽기모드로 연다.
	size = lseek(fd,0,SEEK_END); // 파일의 길이를 잰다.
	printf("%s file size is %d bytes\n",filename,size); // java 파일에 대한 사이즈 출력 
	close(fd);

	fd = open(mainname,O_RDONLY,0644); // main 파일을 읽기 모드로 연다.
	size = lseek(fd,0,SEEK_END); // 파일의 길이를 잰다.
	printf("%s file size is %d bytes\n",mainname,size); // main 파일에 대한 사이즈 출력
	close(fd);
	
	if(strlen(not_mainname) != 0) {
		fd = open(not_mainname,O_RDONLY,0644); // main이 아닌 파일을 읽기모드로 연다.
		size = lseek(fd,0,SEEK_END); // 파일의 길이를 잰다.
		printf("%s file size is %d bytes\n",not_mainname,size); // main이 아닌 파일에 대한 사이즈 출력
		close(fd);
	}
}	
void print_line(char *filename)
{
	int fd;
	int line_cnt=0;
	char buf;
	fd = open(filename,O_RDONLY,0644); // java 파일을 읽기모드로 연다.
	while(read(fd,&buf,1) > 0) { // 파일을 1글자씩 읽으면서 라인수를 측정한다.
		if(buf == '\n')
			line_cnt++;
	}		
	printf("%s line number is %d lines\n",filename,line_cnt); // java 파일에 대한 라인수를 출력한다.
	close(fd);
	line_cnt = 0;
	fd = open(mainname,O_RDONLY,0644); // main c파일을 읽기모드로 연다.
	while(read(fd,&buf,1) > 0) { // 파일을 1글자씩 읽으면서 라인수를 측정한다.
		if(buf == '\n')
			line_cnt++;
	}		
	printf("%s line number is %d lines\n",mainname,line_cnt); // main c파일에 대한 라인수를 출력한다.
	close(fd);
	line_cnt = 0;
	
	if(strlen(not_mainname) != 0) { 
		fd = open(not_mainname,O_RDONLY,0644); // main 파일이 아닌 c 파일을 읽기모드로 연다.
		while(read(fd,&buf,1) > 0) { // 파일을 1글자씩 읽으면서 라인수를 측정한다.
			if(buf == '\n')
				line_cnt++;
		}
		printf("%s line number is %d lines\n",not_mainname,line_cnt); // main이 아닌 c파일에 대한 라인수를 출력한다.
		close(fd);
	}
}
void make_head() // header 파일 생성
{
	int fd;
	char *fname = "header"; // header라는 파일 명으로 생성한다.
	char function_list[8][100]={{"printf #include <stdio.h>\n"}
		,{"scanf #include <stdio.h>\n"}
		,{"exit #include <stdlib.h>\n"}
		,{"fopen #include <stdio.h>\n"}
		,{"fwrite #include <stdio.h>\n"}
		,{"fflush #include <stdio.h>\n"}
		,{"fclose #include <stdio.h>\n"}
		,{"malloc #include <stdlib.h>\n"}};

	if((fd = open(fname,O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) { // header에 파일을 쓴다.
		fprintf(stderr,"File Open Error %s in make_head()",fname);
		exit(1);
	}

	for(int i = 0; i < 8; i++)
		write(fd,function_list[i],strlen(function_list[i])); // function_list를 header에 쓴다.
		
}
