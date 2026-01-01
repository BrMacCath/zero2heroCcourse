#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "file.h"
#include "parse.h"
#include "common.h"
void print_usage(char *argv[]){
    printf("Usage: %s -n -f <databasefile>\n", argv[1]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");
    printf("\t -l - List the employees\n");
    printf("\t -a - (name,address,hours) add an employee\n");
}



int main(int argc, char *argv[]){
	int c = 0;
    char* filepath = NULL;
    char* addstring = NULL;
    bool newfile = false;
    bool list = false;
    int dbfd = -1;

    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees= NULL;

	while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
		switch(c) {
            case 'n':
				newfile = true;
				break;
			case 'f':
				filepath = optarg;
				break;
            case 'a':
				addstring = optarg;
				break;
            case 'l':
                list = true;
                break;
            case '?':
                printf("Unknown option - %c/n",c);
                break;
            default:
                return -1;
		}
	}

    if(filepath == NULL){
        printf("filepath is a required arguement.\n");
        print_usage(argv);

        return 0;
    }

    if(newfile){
        dbfd = create_db_file(filepath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to create database file.\n");
            return -1;
        };
        if (create_db_header(&dbhdr)== STATUS_ERROR){
            printf("Failed to create database header");
            return -1;
        }
    }else{
        dbfd = open_db_file(filepath);
        if(dbfd == STATUS_ERROR){
            printf("Unable to open database file.\n");
            return -1;
        }
        if(validate_db_header(dbfd, &dbhdr) ==STATUS_ERROR){
            printf("Failed to validate database header\n");
            return -1;
        }
    }

    if(read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS ){
        printf("Failed to read employees");
        return 0;
    }
    if(addstring){
        add_employee(dbhdr, &employees,addstring);
    }

    if(list){
        list_employees(dbhdr,employees);
    }

    output_file(dbfd,dbhdr,employees);

	return 0;
}