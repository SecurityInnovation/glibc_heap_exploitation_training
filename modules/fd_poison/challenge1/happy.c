#include <stdlib.h>
#include <stdio.h>

/*
Your own virtual kid service :) 

In order to run with a non-standard version LibC, change the interpreter of the binary. 
*/

struct kid {
	long long age; 
	char name[20];
	void* print_info; // Function pointer
};

typedef struct kid kid;

// A list of pointers to kids
kid* all_kids[10];

// Function declarions
void print_kid_younger(kid* my_kid);
void print_older_kid(kid* my_kid);

// Create a kid. 
kid* create_kid(){
	char tmp[20];	

	kid* my_kid = malloc(sizeof(kid));

	// Get the kids information
	printf("Kids name : ");	
	fgets(my_kid->name,20,stdin);

	printf("Kids age : ");		
	fgets(tmp,20,stdin);
	my_kid->age = atoll(tmp);
	
	// Determine print function based upon age
	if(my_kid->age < 10){
		my_kid->print_info = &print_kid_younger;
	}else{
		my_kid->print_info = &print_older_kid;
	}
	
	return my_kid;
}

kid* edit_kid(kid* my_kid)
{ 
	char tmp[20];

	printf("You damn kids never change... well, now it's time!\n");

	// Get the kids information
	printf("Kids name : ");
	fgets(my_kid->name,20,stdin);

	printf("Kids age : ");
	fgets(tmp,20,stdin);
	my_kid->age = atoll(tmp);
	
	return my_kid;
}

// Remove the kid
void dump_kid(kid* my_kid)
{
	puts("Get out of here you..."); 
	free(my_kid);	
}

void print_kid_younger(kid* my_kid){
	puts("Kids information...");	
	puts("====================");
	printf("Name : %s\n", my_kid->name);	
	printf("Age  : 0x%llx\n", my_kid->age);
	printf("If you're happy and you know it clap your hands...\n");	
}

void print_older_kid(kid* my_kid){
	puts("Kids information...");
	puts("====================");
	printf("Name : %s\n", my_kid->name);
	printf("Age  : 0x%llx\n", my_kid->age);
	printf("Get on the ground you freaking teenager! listen here you son of a *(&&)\n");
}

// Initialization steps: ignore this
void init(){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	clearenv();
}

// Helper function for getting the option
int get_option(){
	char tmp[20];
	
	fgets(tmp,10,stdin);
	return atoi(tmp);
}

// Hit this to win! Easy shell :) 
void win(){
	system("/bin/bash");
}

void banner(){
	// Options..
	puts("");
	puts("Virtual Kid Management");
	puts("=====================================");
	puts("1. Create a kid");
	puts("2. View a kid");
	puts("3. Free a kid");
	puts("4. Change a kid");
	puts("5. Exit");
	puts("6. Options");
	printf(">");
}

int main(){

	int option, second_option; 
	
	// A list of kid pointers
	int counter = 0;
	init();


	while(1){
		banner(); 
		option = get_option();

		// Have a kid
		if(option == 1 && counter < 10){
			all_kids[counter] = create_kid();
			counter += 1;
		}

		// Print a specific kid
		else if(option == 2){
			printf("Kid # : ");
			second_option = get_option();
			
			if(second_option < counter && second_option >= 0){

				// Calls a function pointer
				void (*func_ptr)(struct kid*); 
				func_ptr = all_kids[second_option]->print_info;
				func_ptr(all_kids[second_option]);
			}else {
				puts("Failed to print...");
			}
		}

		// Free a kid
		else if(option == 3){	
			printf("Kid # : ");
			second_option = get_option();

			// Frees the kid if the count is high enough
			if(second_option < counter && second_option >= 0){
				dump_kid(all_kids[second_option]);
			}else{
				printf("Failed to free...");
			}
	
		}
		// Edit a kid
		else if(option == 4){
			printf("Kid # : ");
			second_option = get_option();

			if(second_option < counter && second_option >= 0){
				all_kids[second_option] = edit_kid(all_kids[second_option]);
			}else{
				printf("Failed to edit...");
			}
		}	
		// Exit
		else if(option == 5){
			return 0;
		}
		else{
			puts("Bad Option!"); 
			banner(); 
		}
	}
}
