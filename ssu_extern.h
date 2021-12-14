#ifndef __SSU_EXTERN_H__
#define __SSU_EXTERN_H__

extern char token_list[1000][500]; // 처음 token을 저장하는 배열
extern char token_list2[1000][500]; // 1차 null을 제거한 후 token을 저장하는 배열
extern char token_list3[1000][500]; // 변환을 거친 후 token을 저장하는 배열
extern char function_list[50]; // 함수 list를 저장하는 배열 
extern char mainname[30]; // main이 들어간 파일 이름을 담는 배열
extern char not_mainname[30]; // main이 아닌 파일 이름을 담는 배열
extern char matching_function_list[50][500]; // -p 옵션용 배열

extern int word;
extern int word2;
extern int word3;
extern int function_using[50]; // 함수를 사용한 적이 있는가?

#endif
