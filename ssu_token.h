#ifndef __TOKEN_H__
#define __TOKEN_H__

int divide_token(char *temp); // token 분리 작업
void append(char *des, char c); // 문자 추가 함수
void print_token_list(void);
void tokenizer(char *input);
int null_delete(int w_count,char *source_token_list[500],char *new_token_list[500]);
void token_to_convert(void);
void name_set(void);
void covert_function(void);
void convert_file_write(void);
#endif
