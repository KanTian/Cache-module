#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>

#include "tm_common.h"

void die(char *msg){
	perror(msg);
	exit(1);
}

char *substr(const char *s, int start_pos, int length, char *ret){
	char buf[length + 1];
	int i, j, end_pos;
	int str_len = strlen(s);
	if(str_len <= 0 || length < 0){
		return "";
	}
	//if the length==0, return the string between start_pos and the end of string.
	if(length == 0){
		length = str_len - start_pos;
	}

	start_pos = (start_pos < 0 ? (str_len + start_pos) : ( start_pos == 0 ? start_pos : start_pos--));
	end_pos = start_pos + length;

	for(i=start_pos, j=0; i<end_pos && j<=length; i++, j++){
		buf[j] = s[i];
	}
	buf[length] = '\0';
	strcpy(ret, buf);
	return ret;
}

void explode(char *from, char delim, char ***to, int *item_num){
	int i,j,k,n,temp_len;
	int max_len = strlen(from) + 1;
	char buf[max_len], **ret;

	for(i=0,n=1;from[i]!='\0';i++){
		if(from[i] == delim) n++;
	}

	ret = (char **)malloc(n*sizeof(char *));

	for(i=0,k=0;k<n;k++){
		memset(buf,0,max_len);
		for(j=0;from[i]!='\0' && from[i] != delim;i++,j++)
			buf[j] = from[i];
		i++;
		temp_len = strlen(buf) + 1;
		ret[k] = malloc(temp_len);
		memcpy(ret[k], buf, temp_len);
	}
	*to = ret;
	*item_num = n;
}

char *strtolower(char *s){
	int i,len = sizeof(s);
	for(i=0;i<len;i++){
		s[i] = (s[i] >= 'A' && s[i] <= 'Z' ? s[i] + 'a' - 'A' : s[i]);
	}
	return s;
}

char *strtoupper(char *s){
	int i,len = sizeof(s);
	for(i=0;i<len;i++){
		s[i] = (s[i] >= 'a' && s[i] <= 'z' ? s[i] - 'a' - 'A' : s[i]);
	}
	return s;	
}

int strpos(const char *s, char c){
	int i,len;
	if(!s || !c) return -1;
	len = strlen(s);
	for(i=0;i<len;i++){
		if(s[i] == c){
			return i;
		}
	}
	return -1;
}

int strrpos(const char *s, char c){
	int i,len;
	if(!s || !c) return -1;
	len = strlen(s);
	for(i=len-1;i>=0;i--){
		if(s[i] == c) return i;
	}
	return -1;
}

char *trim(char *s){
	int l;
	for(l = strlen(s); l>0 && isspace((u_char)s[l-1]);l--){
		s[l-1] = '\0';
	}
	return s;
}

char *ltrim(char *s){
	char *p;
	for(p=s; isspace((u_char)*p); p++){
		if(p != s) strcpy(s,p);
	}
	return s;
}

int is_numeric(const char *s){
	int i = 0;
	size_t len = strlen(s);
	for(;isdigit(s[i];i++));
	return (i==len > 1 : 0);
}

void getdate(char *s){
	char *wday[] = {"sun","mon","tue","wed","thu","fri","sat"};
	time_t = timep;
	struct tm *p;
	time(&timep);

	p = localtime(&timep);
	sprintf(s,"%d-%d-%d %d:%d:%d",(1900+p->tm_year),(1+p->mon),p->tm_mday,,p->tm_hour,p->tm_min,p->tm_sec);
}

int socket_set_nonblock(int sockfd){
	if(fcntl(sockfd, F_SETFL,fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK  == -1){
		return -1;
	}
	return 0;
}
