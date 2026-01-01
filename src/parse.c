#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "parse.h"
#include "common.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees,char* addstring){
    printf("%s\n",addstring);

    char *name = strtok(addstring,",");

    char *addr = strtok(NULL,",");

    char *hours = strtok(NULL,",");

    printf("%s %s %s\n",name,addr,hours);

    strncpy(employees[dbhdr->count-1].name,name,sizeof(employees[dbhdr->count-1].name));

    strncpy(employees[dbhdr->count-1].address,addr,sizeof(employees[dbhdr->count-1].address));

    employees[dbhdr->count-1].hours = atoi(hours);

    return STATUS_SUCCESS;
}


int read_employees(int fd,  struct dbheader_t *dbhdr,struct employee_t **employees_out){
    if(fd <0){
        printf("Got a bad fd from user.\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;
    struct employee_t *employees = calloc(count,sizeof(struct employee_t));
    if( employees == NULL){
        printf("Malloc failed");
        return STATUS_ERROR;
    }

    read(fd,employees, count*sizeof(struct employee_t));

    int i = 0;

    for(;i < count;i++){
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employees_out = employees;

    return STATUS_SUCCESS;


}


int output_file(int fd, struct dbheader_t *dbhdr,struct employee_t *employees){
     if(fd <0){
        printf("Got a bad fd from user.\n");
        return STATUS_ERROR;
    }
    int realcount = dbhdr->count;

    dbhdr->magic =htonl(dbhdr->magic);
    dbhdr->filesize =htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t)*realcount) );
    dbhdr->count =htons(dbhdr->count);
    dbhdr->version =htons(dbhdr->version);
    lseek(fd,0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    int i=0;
    
    for(; i < realcount;i++){
        employees[i].hours = ntohl(employees[i].hours);
        write(fd,&employees[i],sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}

int validate_db_header(int fd,  struct dbheader_t **header_out){
    if(fd <0){
        printf("Got a bad fd from user.\n");
        return STATUS_ERROR;
    }
    
    struct dbheader_t *header = calloc(1,sizeof(struct dbheader_t));
    
    if (header == NULL){
        printf("Malloc failed to create a db header.\n");
        return STATUS_ERROR;
    }
    
    if (read(fd,header,sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version =ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    if(header->magic != HEADER_MAGIC){
        printf("Improper header magic");
        free(header);
        return -1;
    }

    if(header->version != 1){
        printf("Improper header version");
        free(header);
        return -1;
    }

    struct stat dbstat = {0};
    fstat(fd,&dbstat);
    if(header->filesize != dbstat.st_size){
        printf("Corrupted database\n");
        free(header);
        return -1;
    }
    *header_out = header;
    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **header_out){
    struct dbheader_t *header = calloc(1,sizeof(struct dbheader_t));

    if(header ==NULL){
        printf("Malloc failed to create db header.\n");
        return STATUS_ERROR;
    }

    header->version = 0x1;
    header->count= 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *header_out = header;

    return STATUS_SUCCESS;
}

