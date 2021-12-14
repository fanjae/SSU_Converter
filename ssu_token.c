#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include "ssu_extern.h"

char token_list[1000][500];
char token_list2[1000][500];
char token_list3[1000][500];
char function_list[50];
char mainname[30];
char not_mainname[30];
char mainname_temp[30];
char notmain_temp[30];
char matching_function_list[50][500]={{"System.out.printf() -> printf()"},{"scn.nextInt() -> scanf()"},{"FileWriter() -> fopen()"},{"write() -> fwrite()"},{"flush() -> fflush()"},{"close() -> ffclose()"}};
char extern_write[50][500];

int word,word2,word3;
int function_using[50]={0};
int using_head[10]={0};
int extern_word;

void convert_function(void);
void convert_file_write(char *filename);

int divide_token(char temp) // token을 나누는 과정을 하는 함수이다. '='을 제외한 사칙 연산 및 괄호등의 함수는 1을 리턴함.
{
	if(temp == '+' || temp == '-' || temp == '*' || temp == '/' || temp == '<' || temp == '(' || temp == '{' || temp == '>' || temp == '}' || temp == ')' || temp == ',' || temp == ';')
	{
		return 1;
	}
	else if(temp == '=') // '='은 2를 리턴한다.
	{
		return 2;
	}
	else
	{
		return 0;
	}
}
void append(char *des, char c) // 문자열 끝에 문자를 추가하는 함수이다.
{
	char *p = des;
	while (*p != '\0') p++;
	*p = c;
	*(p+1) = '\0';
}
void tokenizer(char *input) // token을 나누는 함수이다.
{
	char list[100]={0};
	char temp_word[100]={0};
	int index = 0;
	strcpy(list,input); // 1줄 단위로 읽어서 처리한다.
	while(1)
	{

		if(divide_token(list[index+1]) == 1) { // token을 나눌 때 다음 위치 값이 '=' 외에 나눠야할 값인 경우

			append(temp_word,list[index]); // temp_word에 list 값을 넣는다.
			strcat(token_list[word],temp_word); // token_list에 temp_word 값 추가
			temp_word[0] = 0;
			word++; // 단어 개수 추가
			index++; // 길이 추가
			continue;

		}

		if(divide_token(list[index]) == 1) { // token 나눌 때 현재 위치 값이 '='외에 나눠야할 값인 경우
			if(divide_token(list[index+1]) > 1) { // 추가적으로 '='인 경우

				append(temp_word,list[index]); // temp_word에 list값을 넣는다.
				append(temp_word,list[index+1]); // temp_word에 다음값도 추가.
				strcat(token_list[word],temp_word); // token_list에 추가
				temp_word[0] = 0;
				word++;
				index += 2;
				continue;

			}
			// '='가 아닌 경우
			append(temp_word,list[index]); // temp_word에 list값을 넣는다.
			strcat(token_list[word],temp_word); // token_list에 다음 값도 추가한다.
			temp_word[0] = 0;
			word++;
			index++;
			continue;

		}

		if(list[index] == 9 || list[index] == 10 || list[index] == 0 || list[index] == 32) { // 스페이스, 탭, 엔터에 대한 처리
			strcat(token_list[word],temp_word); // token_list에 해당 값에 대해서 별도로 처리한다.
			temp_word[0] = 0;
			word++;
			if(list[index] != 0) { 
				append(temp_word,list[index]); // temp_word에 list값을 넣는다. 
				strcat(token_list[word],temp_word); // token_list에 다음값도 추가한다.
				temp_word[0] = 0;
				word++;
				index++;
			}
			else if(list[index] == 0) // list[index]가 null값이면 반복문을 나간다.
				break;

		}
		else {

			append(temp_word,list[index]); // temp_word에 list 값을 넣는다. 
			index++;
			continue;

		}
	}
	
}
int null_delete(int w_count,char source_token_list[][500],char new_token_list[][500]) // token에 들어간 NULL 문자를 빼고 새로 담음.
{
	int num = 0;
	for(int i = 0; i < w_count; i++) { 
		if(strcmp(source_token_list[i],"") == 0) // token_list안에 있는 값이 NULL로 들어가 있는 경우에는 새로운 token_list에서는 제외시킨다.
			continue;
		strcat(new_token_list[num++],source_token_list[i]);
	} 
	return num;
}
void token_to_convert(void) // token에 대한 재 변환.(1차 변환)
{
	for(int i = 0; i < word; i++) { 
		if(strcmp(token_list[i],"void") == 0) {
			if(strcmp(token_list[i+2],"main") == 0) {
				token_list[i][0] = 0;
				token_list[i+4][0]=0;
				token_list[i+5][0]=0;
				token_list[i+6][0]=0;	
				strcat(token_list[i],"int");
				strcat(token_list[i+4],"void");
			}
		}
	/* 위 문장은 void main(String[] args)와 같은 문장을 처리 */
		if(strcmp(token_list[i],"Scanner") == 0) { // Scanner에 대한 무시.
			token_list[i][0] = 0;
			
			for(int j=i-1; j>=0; j--) { // 엔터값을 만나기 전까지 모든 해당 문장 앞부분 모두 제거
				if(strcmp(token_list[j],"\n") == 0) {
					break;
				}
				else
					token_list[j][0] = 0;
			} 
			for(int j=i+1; ; j++) { // 엔터값 만나기 전까지 모든 해당 문장 뒷부분 모두를 제거한다.
				if(strcmp(token_list[j],"\n") == 0) {
					token_list[j][0] = 0;
					i = j; 
					break;
				}
				else
					token_list[j][0] = 0;
			}
		}
		if(strcmp(token_list[i],"import") == 0) { // import에 대한 무시
			token_list[i][0] = 0;
		// import 문장이 발생하면 엔터값을 만날때까지 해당 뒷부분 모두를 제거한다.	
			for(int j=i+1; ; j++) {
				if(strcmp(token_list[j],"\n") == 0) { 
					token_list[j][0] = 0;
					i = j; 
					break;
				}
				else
					token_list[j][0] = 0;
			}

		}
		if(strcmp(token_list[i],"/") == 0) { // 한줄 단위 주석에 대한 처리 '//' 주석에 인정 기준은 엔터값을 만나기전까지로 본다. 그 이전까지는 모두 합친다.
			if(strcmp(token_list[i+1],"/") == 0) {
				strcat(token_list[i],token_list[i+1]);
				token_list[i+1][0] = 0;
				for(int j=i+2; ; j++) {
					if(strcmp(token_list[j],"\n") == 0) {
						i = j;
						break;
					}
					else { 
						strcat(token_list[i],token_list[j]);
						token_list[j][0] = 0;
					}
				}
			}
			if(strcmp(token_list[i+1],"*") == 0) { // 여러 줄 단위 주석에 대한 처리 '/*' 주석에 대한 인정 기준은 */를 만나기전까지로 본다. 그 이전까지는 모두 같은 문장으로 간주하여 합친다.
				strcat(token_list[i],token_list[i+1]);
				token_list[i+1][0] = 0;
				for(int j=i+2; ; j++) {
					if((strcmp(token_list[j],"*") == 0) && (strcmp(token_list[j+1],"/") == 0)) {
						strcat(token_list[i],token_list[j]);
						strcat(token_list[i],token_list[j+1]);
						token_list[j][0] = 0;
						token_list[j+1][0] = 0;
						i = j;
						break;
					}	
					else {
						strcat(token_list[i],token_list[j]);
						token_list[j][0] = 0;
					}
				}
			}
		}
		// 엔터값이 있는 경우 해당 문장의 다음 문장 탭 하나를 제거한다.
		if(strcmp(token_list[i],"\n") == 0) {
			for(int j=i+1; j < word; j++) {
				if(strcmp(token_list[j],"\t") == 0) {
					token_list[j][0] = 0;
					i = j;
					break;
				}
			}
		}
		// scn.nextInt라는 문장이 나왔으면 해당 문장 뒤 괄호까지 한문장으로 간주하여 합친다.
		if(strcmp(token_list[i],"scn.nextInt") == 0) {
			strcat(token_list[i],token_list[i+1]);
			strcat(token_list[i],token_list[i+2]);
			token_list[i+1][0] = 0;
			token_list[i+2][0] = 0;
		}
	}
	word2 = null_delete(word,token_list,token_list2);
}
void name_set(void)
{
	int index;
	for(int i=0; i<word2; i++) {
	//	class 문장안에 main이 있는 경우 없는 경우를 찾는다.
		if(strcmp(token_list2[i],"class") == 0) {
			index = i;
			if(strcmp(token_list2[index+11],"main") == 0) {
				strcat(mainname_temp,token_list2[i+2]);
				// main을 찾은 경우 main 파일이름을 정한다.
			}
			else if(strcmp(token_list2[index+12],"main") == 0) {
				strcat(mainname_temp,token_list2[i+2]);
				// main을 찾은 경우 main 파일이름을 정한다.
			}
			else {
				strcat(notmain_temp,token_list[i+2]);
				// main이 아닌 경우 not_main 파일이름을 정한다.
			}
		}

	}
	strcat(mainname,mainname_temp);
	strcat(mainname,".c"); // main이 들어있는 파일의 이름뒤에 .c를 붙인다.

	if(strlen(notmain_temp) != 0) {
		strcat(not_mainname,notmain_temp);
		strcat(not_mainname,".c"); // main이 들어있지 않은 파일의 이름 뒤에 .c를 붙인다.
	}
	convert_function(); // 함수를 변환한다. 
	convert_file_write(mainname); // 변환한 파일을 쓴다.
}

void convert_function(void)
{
	char final_name[2][100]={0};
	char constructor_name[100]={0};
	if(strlen(not_mainname) != 0) { // main 파일이 없는 경우 처음 나오는 '{'부터 맨앞까지 모두 NULL값으로 처리한다.
		for(int i = 0; i < word2; i++) {
			if(token_list2[i][0] == '{') {
				for(int j = i; j >= 0; j--) 
					token_list2[j][0] = 0;
				break;
			}
		}
	}
	for(int i = 0; i < word2; i++) {
		// main이 있는 경우 있는 경우 q2와 같은 상황에 대해서 main 이전부터 '}'까지 모든 문장을 NULL로 바꾼다.
		if(strcmp(token_list2[i],"main") == 0) {
			for(int j = i-3; j>=0; j--) {
				if(strcmp(token_list2[j], "}") == 0) {
					token_list2[j][0] = '\n';
					break;
				}
				token_list2[j][0] = 0;
			}
		}
		// 생성자 문장을 public void로 우선 바꿔서 q2에 넣을때 함수 처리를 하도록 만든다.
		if((strcmp(token_list2[i],"public") == 0) && (strcmp(token_list2[i+2],notmain_temp) == 0)) {
			token_list2[i][0] = 0;
			strcat(token_list2[i],"public void");
		}
		// q2.java에 있는 int[]와 같은 상황을 포인터로 변환한다.
		if(strcmp(token_list2[i],"int[]") == 0) {
			token_list2[i][0] = 0;
			strcpy(token_list2[i],"int*");
		}
		
		// final이 나온 경우 final이 나온 이후 가장 첫번째로 나온 =값에 대한 이전 값과 이후 값을 저장한다.
		if(strcmp(token_list2[i],"final") == 0) {
			for(int j = i+1; ; j++) {
				// '=' 이전 값 및 이후 값을 저장한다.
				if(strcmp(token_list2[j],"=") == 0) {
					for(int k = j-1; ; k--) {
						if(token_list2[k][0] != 32) { 
							strcpy(final_name[0],token_list2[k]);
							break;
						}
					}
					for(int k = j+1; ; k++) {
						if(token_list2[k][0] != 32) {
							strcpy(final_name[1],token_list2[k]);
							break;
						}
					}
				}
				// final int STACK_SIZE = 10; 인 경우
				// STACK_SIZE와 10을 저장하는 것이다.
				// ';'가 나온 경우 반복문을 끝낸다.
				if(strcmp(token_list2[j],";") == 0)
					break;
			}
		
			// final이 나왔을때 이전값에 대하여 엔터값이 나오기전까지 모든 문장을 지운다.
			for(int j = i-1; ; j--) {
				if(strcmp(token_list2[j],"\n") == 0) 
					break;
				token_list2[j][0] = 0;
			}
		
			// final이 나왔을때 이후값에 대하여 엔터값 이후에 세미콜론을 만났을때까지 모든 문장을 지운다.(세미콜론도 지운다.) 
			for(int j = i+1; ; j++) {
				if(strcmp(token_list2[j],";") == 0) {
					token_list2[j][0] = 0;
					break;
				}
				token_list2[j][0] = 0;
			}
			// #define으로 변환하는 과정을 거친다.
			sprintf(token_list2[i],"#define %s %s",final_name[0],final_name[1]);
			
		}

		// 생성자 문장을 함수로 바꾼다. public Stack() -> void Stack()
		if((strcmp(token_list2[i],"public") == 0) && (strcmp(token_list2[i+2],notmain_temp) == 0)) {
			token_list2[i][0] = 0;
			strcpy(token_list2[i],"void");
		}
		// new를 찾은 경우 malloc를 하여 처리 한다.  
		// Stack은 본 문장을 이용하여 처리.
		if((strcmp(token_list2[i],"new") == 0)) {
			char find_set[50]={0};
			char temp_name[4][100] = {0};
			strcat(find_set,"int");
			strcat(find_set,"[");
			strcat(find_set,final_name[0]);
			strcat(find_set,"]");
			if(strcmp(token_list2[i+2],find_set) == 0) {
				for(int j = i; ; j++) {
					if(strcmp(token_list2[j],";") == 0)
						break;
					token_list2[j][0] = 0;
				}
				sprintf(token_list2[i],"malloc(sizeof(int) * %s)",final_name[0]);
				if(using_head[7] == 0)
					using_head[7]++;
			}	
		}
		// System.out.printf() -> printf()로 처리한다.
		if(strcmp(token_list2[i],"System.out.printf") == 0) {
			token_list2[i][0] = 0;
			strcpy(token_list2[i],"printf");
			if(using_head[0] == 0) { // System.out.printf()가 사용된 경우 헤더 및 함수를 사용했다고 알림.
				using_head[0]++;
				function_using[0]++;
			}
		}

		// scn.nextInt() -> scanf()로 처리한다.
		if(strcmp(token_list2[i],"scn.nextInt()") == 0) {
			int off_set;
			int off_set_end;
			char temp_name[500];
			for(int j = i-1; j>=0; j--) {
				// '=' 을 만난 경우
				if(strcmp(token_list2[j],"=") == 0) {
					off_set = j;
					break;
				}
			}
			// =을 만나기 이전 값을 추적하여  스페이스 값이나 탭값이 아닌 경우 num = scn.nextInt() --> num 저장.
			for(int j = off_set-1; j >= 0; j--) {
				if(token_list2[j][0] != 32 && token_list2[j][0] != 9) {
					strcpy(temp_name,token_list2[j]);
					off_set_end = j;
					break;
				}
			}
			// num = scn.nextInt()라고 볼때 scn.nextInt() 부터 num이전 까지 모든 값 제거
			for(int j = i; j > off_set_end; j--) {
				token_list2[j][0] = 0;
			}

			// scanf로 바꾼 값 출력
			strcpy(token_list2[off_set_end],"scanf(\"%d\",&");
			strcat(token_list2[off_set_end],temp_name);
			strcat(token_list2[off_set_end],")");
			if(using_head[1] == 0) { // scn.nextInt()가 사용된 경우 헤더파일 및 함수를 사용했음을 알려줌
				using_head[1]++;
				function_using[1]++;
			}
		}

		// return -> exit(0)로 토큰을 바꾼다.
		if(strcmp(token_list2[i],"return ") == 0) {
			token_list2[i][0] = 0;
			strcpy(token_list2[i],"exit(0)");
			for(int j = i+1; ; j++) {
				// 세미콜론이 나타난 경우 반복문을 끝낸다. 
				if(strcmp(token_list2[j],";") == 0) {
					i = j; 
					break;
				}
				token_list2[j][0] = 0;
			}
			if(using_head[2] == 0)
				using_head[2]++;
		}
		// c에서는 IOException이라는 것이 따로 존재하지 않으므로 해당 문장은 없애도록 처리한다.
		if(strcmp(token_list2[i],"IOException") == 0) {
			for(int j = i; ; j--) {
				if(strcmp(token_list2[j],")") == 0) 
					break;
				token_list2[j][0] = 0;
			}
		}
		// File이 나온경우 
		if(strcmp(token_list2[i],"File") == 0) {
			char name_temp[5][500];
			int set = 0;
			for(int j = i+1; ; j++) {
				// 스페이스값이 아닌 첫번째 값을 저장한다.
				// 스페이스 값이 아닌 경우 해당값이 파일 경로를 저장할 변수값이 됨.
				if(token_list2[j][0] != 32 && set == 0) {
					strcpy(name_temp[0],token_list2[j]);
					set = 1;
				}
				// 쌍따옴표가 나온 경우 파일 경로로 저장한다.
				if(token_list2[j][0] == '\"')
					strcpy(name_temp[1],token_list2[j]);	
					if(strcmp(token_list2[j],";") == 0)
					break;
			}
			for(int j = i+1; ; j++) {
				// 세미콜론이 나오기 전까지 모든 토큰을 제거한다.
				if(strcmp(token_list2[j],";") == 0) {
					i = j;
					break;
				}
				else 
					token_list2[j][0] = 0;
			token_list2[i][0] = 0;
			// 파일 경로를 지정하는 변수를 추가한다.
			strcpy(token_list2[i],"char *");
			strcpy(token_list2[i+1],name_temp[0]);
			strcpy(token_list2[i+2], " = ");
			strcpy(token_list2[i+3],name_temp[1]);
			}
		}
		if(strcmp(token_list2[i],"FileWriter") == 0) {
			// FileWriter 가 나온 경우 
			char name_temp[5][500];
			int set = 0;
			int set2 = 0;
			for(int j = i+1; ; j++) {
				if(token_list2[j][0] != 32 && set == 0) {
					// FileWriter 다음으로 나오는 변수값을 저장한다.(파일 포인터 변수로 쓰일 것이기 때문)
					strcpy(name_temp[0],token_list2[j]);
					set = 1;
				}
				if(token_list2[j][0] == '=') {
					for(int k = j-1; ; k--) {
						if(token_list2[k][0] != 32) {
							strcpy(name_temp[1],token_list2[k]);
							break;
						}
					}
				}
				// 다시 한번 FileWriter가 나온 경우 
				if(strcmp(token_list2[j],"FileWriter") == 0) {
					for(int k = j+1; ; k++) {
						// ',' 이전 값과 이후 값을 찾아서 넣는다.
						if(token_list2[k][0] == ',') {
							for(int l = k-1; ; l--) {
								if(token_list2[l][0] != 32) {
									strcpy(name_temp[2],token_list2[l]);
									break;
								}
							}
							for(int l = k+1; ; l++) {
								if(token_list2[l][0] != 32) {
									strcpy(name_temp[3],token_list2[l]);
									break;
								}
							}
						}
						// 세미콜론까지 비교해 봤을때 문장이 끝난 경우 반복문 종료(2번째로 나온 FileWriter 에 대한 반복문 종료.
						if(strcmp(token_list2[k],";") == 0)
							break;	
					}
				}
				// 세미콜론이 나온 경우 반복문 종료
				if(strcmp(token_list2[j],";") == 0) 
					break;
			}
			
			// 세미콜론이 나올때까지 해당 문장내의 필요없는 문장들을 NULL값으로 넣어 제거한다.
			for(int j = i; ; j++) {
				if(strcmp(token_list2[j],";") == 0) {
					i = j;
					break;
				}
				else
					token_list2[j][0] = 0;
			}
			char temp_mode;
			// FileWriter에 2번째 값이 false냐 true에 따라서 파일 mode값을 다르게 처리한다.
			if(strcmp(name_temp[3],"false") == 0)
				temp_mode = 'w';
			else 
				temp_mode = 'a';
			sprintf(token_list2[i], "FILE *%s;\n\t",name_temp[1]);
			sprintf(token_list2[i+1],"if ((%s = fopen(%s, \"%c\")) == NULL) {\n\t\tfprintf(stderr,\"fopen error for %%s\\n\", %s);\n\t\texit(1);\n\t}",name_temp[1],name_temp[2],temp_mode,name_temp[2]);
			// FILE에 대한 처리와 fopen을 만든다.
			if(using_head[3] == 0) { // FileWriter이 사용된 경우 헤더 파일 및 함수 사용을 알림.
				using_head[2]++; // 예외적으로 exit를 처리하기 위함
				using_head[3]++;		
				function_using[2]++;
			}	

			for(int j = i+1; j <word2 ; j++) {
				char find_temp[100]={0};
				char find_temp2[100]={0};
				char find_temp3[100]={0};
				char temp_text[500]={0};
				sprintf(find_temp,"%s.write",name_temp[1]);
				sprintf(find_temp2,"%s.flush",name_temp[1]);
				sprintf(find_temp3,"%s.close",name_temp[1]);

				//  token_list를 비교하여 write함수를 썼던 적이 있다면 write 이후에 쓰인 내용을 저장하여 fwrite로 변환한다.
				if(strcmp(token_list2[j],find_temp) == 0) {
					for(int k = j+1; ; k++) {
						if(strcmp(token_list2[k],";") == 0)
							break;
						if(token_list2[k][0] == '(' || token_list2[k][0] == ')') 
							continue;
						strcat(temp_text,token_list2[k]);
					}
					for(int k = j; ; k++) {
						// 세미콜론을 이전까지의 모든 문장 NULL값 부여
						if(strcmp(token_list2[k],";") == 0)
							break;
						token_list2[k][0] = 0;
					}
					sprintf(token_list2[j],"fwrite(%s, sizeof(%s), 1, %s)",temp_text,temp_text,name_temp[1]);
					if(using_head[4] == 0) { // write함수가 사용된 경우 헤더 파일 및 함수 사용을 알린다. 
						using_head[4]++;
						function_using[3]++;
					}
				}
				if(strcmp(token_list2[j],find_temp2) == 0) { // flush 함수가 사용된 경우	
					for(int k = j; ; k++) {
						if(strcmp(token_list2[k],";") == 0) // 세미콜론 이전까지의 모든 문장 NULL값 처리
							break;
						token_list2[k][0] = 0;
					}
					sprintf(token_list2[j],"fflush(%s)",name_temp[1]);
					if(using_head[5] == 0) { // flush함수가 사용된 경우 헤더 파일 및 함수 사용을 알린다.
						using_head[5]++;
						function_using[4]++;
					}
				}
				if(strcmp(token_list2[j],find_temp3) == 0) { // close 함수가 사용된 경우
					for(int k = j; ; k++) { // 세미콜론 이전까지의 모든 문장 NULL값 처리 
						if(strcmp(token_list2[k],";") == 0)
							break;
						token_list2[k][0] = 0;
					}
					sprintf(token_list2[j],"fclose(%s)",name_temp[1]);
					if(using_head[6] == 0) { // close함수가 사용된 경우 헤더 파일 및 함수 사용을 알린다.
						using_head[6]++;
						function_using[5]++;
					}
				}
			}
		}

		// public void이거나 public이 들어간 경우(method로 간주) 
		if((strcmp(token_list2[i],"public void") == 0) || (strcmp(token_list2[i],"public") == 0)) {
			if(strcmp(token_list2[i],"public void") == 0) {
				// public void인 경우에는 void로 바꾸고 main에 extern을 넣기 위해 처리한다.
				token_list2[i][0] = 0;
				token_list2[i+1][0] = 0;
				strcpy(token_list2[i],"void ");
				sprintf(extern_write[extern_word++],"extern %s%s();\n",token_list2[i],token_list2[i+2]);
			}
			else if((strcmp(token_list2[i+2],"static") != 0) && (strcmp(token_list2[i+2],"class") != 0)) {
				// class가 아니면서 public이 들어가는 경우 main에 extern을 넣기 위해 처리한다.
				sprintf(extern_write[extern_word++],"extern %s %s();\n",token_list2[i+2],token_list2[i+4]);
				token_list2[i][0] = 0;
				token_list2[i+1][0] = 0;
			}
		}
		if((strcmp(token_list2[i],"null") == 0)) { // null이라는 값이 들어가는 경우 NULL로 바꾸어 처리한다.
			token_list2[i][0] = 0;
			strcpy(token_list2[i],"NULL");
		}
		if(strlen(notmain_temp) != 0) { // main과 main이 아닌 것이 존재하는 경우 
			if(strcmp(token_list2[i],"main") == 0) {
				int find = 0;
				for(int j = i+1; ; j++) {
					// 생성자 이름을 저장하기 위해 해당 문장에 있는 constructor name을 찾는다. 
					if(strcmp(token_list2[j],notmain_temp) == 0 && find == 0) {
						for(int k = j+1; ; k++) {
							if((token_list2[k][0] != ' ') && find == 0) {
								strcpy(constructor_name,token_list2[k]);
								append(constructor_name,'.');
								find = 1;
								break;
							}
						}
					}
					// 세미콜론이 나온 경우 반복문을 나간다.
					if(strcmp(token_list2[j],";") == 0)
						break;
				}
				
				for(int j = i+1; ; j++) {
					// c언어는 생성자가 없으므로 함수를 호출하는 형태 처럼 바꾼다.
					if(strcmp(token_list2[j],notmain_temp) == 0) {
						for(int k = j; ; k++) {
							if(strcmp(token_list2[k],";") == 0) {
								sprintf(token_list2[k],"%s();",notmain_temp);
								break;
							}
							token_list2[k][0] = 0;
						}
					}
					// 세미콜론이 나온 경우 반복문을 나간다.
					if(strcmp(token_list2[j],";") == 0)
						break;
				}
			}

			int correct = 0;
			int constructor_len = strlen(constructor_name);
			char temp[100]={0};
			// main 이외 다른 클래스에서 만들어졌던 모든 함수들에 앞에 있는 constructor name을 제거한다.
			for(int j=0; j < constructor_len; j++) {
				if(token_list2[i][j] == constructor_name[j]) {
					correct++;
				}
				if(correct == constructor_len) {
					int token_len = strlen(token_list2[i]);
					for(int j=correct; j < token_len; j++)
						append(temp,token_list2[i][j]);
					token_list2[i][0] = 0;
					sprintf(token_list2[i],"%s",temp);
				}
			}
		}						
	}
	// 토큰 리스트에 마지막 중괄호를 제거한다.
	for(int i = word2-1; i >= 0; i--) {
		if(strcmp(token_list2[i],"}") == 0) {
			token_list2[i][0] = 0;
			break;
		}
		token_list2[i][0] = 0;
	}
	word3 = null_delete(word2,token_list2,token_list3);	
}
void convert_file_write(char *filename) // java파일에 있는 변환한 결과를 파일을 씀.
{
	char temp_header[8][100]={0};
	int fd;
	int fd2;
	int head_fd;
	if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) { // main에 쓸 파일을 연다.
		fprintf(stderr, "File Open Error %s in convert_file_write()",filename);
		exit(1);
	}
	if((head_fd = open("header", O_RDONLY, 0644)) < 0) { // headerfile을 연다.
		fprintf(stderr, "File Open Error %s in convert_file_write()","header");
		exit(1);
	}

	int head_word = 0;
	
	if(strlen(not_mainname) != 0) { // main이 아닌 파일에 대해서 쓰기 모드로 연다.
		if((fd2 = open(not_mainname, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
			fprintf(stderr, "File Open Error %s in convert_file_write()",not_mainname);
			exit(1);
		}	
	}

	char head_temp;
	int head_set_len[8]={0};
	int head_len = 0;
	while(1) { // header 파일에 있는 내용을 불러온다. 
	
		if (read(head_fd, &head_temp, 1) > 0) {
			if(head_temp == 0)
				continue;
			if(head_temp == 10) {
				temp_header[head_word][head_len+1] = 0;
				head_word++;
				head_len = 0;
			}
			temp_header[head_word][head_len] = head_temp;
			head_len++;
		}
		else
			break;
	}

	int header_on[2]={0};
	int write_on[2]={0};
	char set_match_fun[2][100]={0};
	char set_write_fun[2][100]={0};
	char function_list[8][20]={{"printf"},{"scanf"},{"exit"},{"fopen"},{"fwrite"},{"fflush"},{"fclose"},{"malloc"}};
	
	// header파일에 있는 함수 내용을 가지고 와서 #include <stdio.h>, #include <stdlib.h>등을 가지고 온다.
	// 이 과정에서 헤더가 중복되는 경우 제외시킨다.
	for(int i = 0; i < 8; i++) {
		int save_head=0;
		char my_function[50] = {0};
		char my_header[50] = {0};
		if(using_head[i] > 0) {
			for(int j = 0; ; j++) {
				if(save_head == 0 && temp_header[i][j] == ' ') {
					save_head = 1;
					continue;
				}
				else if(save_head == 1 && (temp_header[i][j] == '\n' || temp_header[i][j] == 0)) {
					append(my_function,'\n');
					break;
				}
				else if(save_head == 1 && (temp_header[i][j] != '\n' && temp_header[i][j] != 0)) {
					append(my_function,temp_header[i][j]);
				}
				else  
					append(my_header,temp_header[i][j]);
			}
			if((strcmp("#include <stdio.h>\n",my_function) == 0) && (write_on[0] == 0)) {
				write_on[0] = 1;
			}
			else if((strcmp("#include <stdlib.h>\n",my_function) == 0) && (write_on[1] == 0)) {
				write_on[1] = 1;
			}
		}
	}
	if(write_on[0] == 1) { // #include <stdio.h> 에 대한 처리
		write(fd,"#include <stdio.h>\n",strlen("#include <stdio.h>\n"));
		if(strlen(not_mainname) != 0) {
			write(fd2,"#include <stdio.h>\n",strlen("#include <stdio.h>\n"));
		}
	}

	if(write_on[1] == 1) { // #include <stdlib.h> 에 대한 처리
		write(fd,"#include <stdlib.h>\n",strlen("#include <stdlib.h>\n"));
		if(strlen(not_mainname) != 0) {
			write(fd2,"#include <stdlib.h>\n",strlen("#include <stdlib.h>\n"));
		}
	}

	int index = 0;
	if(strlen(not_mainname) != 0) {
		for(int i = 0; i < word3; i++)	{ 
			// main 위치를 저장한다.
			if((strcmp(token_list3[i],"int") == 0) && (strcmp(token_list3[i+2],"main") == 0))
				index = i;			
		}
	}
	if(index == 0) {
		// token 개수 만큼 파일을 쓴다.
		for(int i = 0; i < word3; i++) {
			char temp[500]={0};
			strcpy(temp,token_list3[i]);
			write(fd,temp,strlen(temp));
		}
		printf("%s convert Success!\n",mainname);
	}
	else {
		// main 위치가 저장되었었다면 해당 파일은 반드시 main만 있는 파일이 아니었을 것이다. extern_word에 있는 내용을 쓴다.
		for(int i = 0; i < extern_word; i++) {
			write(fd,extern_write[i],strlen(extern_write[i]));
		}
		int enter_set;
		// 파일을 쓴다. 이때 main이냐 아니냐에 따라서 서로 다른 파일에 쓰도록 설정한다.
		for(int i = 0; i < word3; i++) {
			char temp[500]={0};
			strcpy(temp,token_list3[i]);
			if(i >= index-1) {
				if(token_list3[i][0] == '\n' && enter_set != 1) {
					enter_set = 1;
					continue;
				}
				write(fd,temp,strlen(temp));
			}
			else
				write(fd2,temp,strlen(temp));
		}
		printf("%s convert Success!\n",mainname);
		printf("%s convert Success!\n",not_mainname);
	}
	
	int make_fd;
	char make_temp[100]={0};
	strcat(make_temp,mainname_temp);
	strcat(make_temp,"_Makefile");
	// 메이크 파일을 만들어 줄 것에 대한 파일을 쓰기 모드로 연다.
	if((make_fd = open(make_temp,O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
		fprintf(stderr,"File Open Error for %s\n",make_temp);
		exit(1);
	}
	
	char make_set[500];
	if(strlen(not_mainname) == 0) {
	// makefile을 만드는 과정이다. (main만 있는 경우)
		sprintf(make_set,"%s: %s.o\n",mainname_temp,mainname_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"\tgcc -g -o %s %s.o\n\n",mainname_temp,mainname_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"%s.o: %s.c\n",mainname_temp,mainname_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"\tgcc -c -o %s.o %s.c",mainname_temp,mainname_temp);	
		write(make_fd, make_set,strlen(make_set));
	}
	else {
	// makefile을 만드는 과정이다. (파일이 2개인 경우)
		sprintf(make_set,"%s: %s.o %s.o\n",mainname_temp,mainname_temp,notmain_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"\tgcc -o %s %s.o %s.o\n\n",mainname_temp,mainname_temp,notmain_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"%s.o: %s.c\n",mainname_temp,mainname_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"\tgcc -c -o %s.o %s.c\n\n",mainname_temp,mainname_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"%s.o: %s.c\n",notmain_temp,notmain_temp);
		write(make_fd, make_set,strlen(make_set));
		sprintf(make_set,"\tgcc -c -o %s.o %s.c\n",notmain_temp,notmain_temp);
		write(make_fd, make_set,strlen(make_set));
	}
}
