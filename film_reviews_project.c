//Thanasis Makrygiannis 2015/2016 project in C
//Application for evaluation of film reviews

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 100


//node struct, nodeT
struct list {
	struct list *next; //*next point to next node
	char *word; //*word points to review words
	int num_app;
	float score;
};
typedef struct list nodeT;


//hash table struct, hashT
typedef struct {
	int size;
	int num_entries;
	nodeT **table; //**table -> pointer to head of list
} hashT;

//a global pointer to a hash table
hashT *hash_head;

//functions
void init_hash_table();
char *read_next_line(char *filename);
void string_into_tokens(char *read_from);
unsigned long hash(char *word);
void add_nodeT(nodeT *head, char *word, float score, unsigned long position);
void insert_hashTable(char *word, float score);
void print_nodeT_list(nodeT *head);
void destroy_nodeT_list(nodeT *head, int position);
nodeT *extract_nodeT(nodeT *head, int i);
void print_hashTable();
void destroy_hashT();
nodeT *best_score(nodeT *head);
nodeT *find_best(nodeT *head, int position);
void transfer_node(char *word, nodeT *extracted_nodeT);
void rehashing ();
void read_reviews();


//INITIALIZE A HASH TABLE
void init_hash_table() {

	//allocate memory for hashT
	hash_head = malloc(sizeof(hashT));
	if (!hash_head) {
		printf("Error allocating memory.\n");
		exit(0);
	}

	hash_head->size = HASH_SIZE;
	hash_head->num_entries = 0;

	//allocate memory for table[i] of hashT
	hash_head->table = malloc(sizeof(nodeT)*hash_head->size);
	if (!hash_head->table) {
		printf("Error allocating memory.\n");
		exit(0);
	}

	int i;
	for (i=0; i<hash_head->size; i++) {
		hash_head->table[i] = NULL;   //initialize hash table with NULL
	}
}

//READS LINE FROM FILE
char *read_next_line(char *filename) {

	static int firstrun = 1;
	static FILE *fp;

	if (firstrun) {
		fp = fopen(filename, "r");
		if (fp == NULL) {
			fprintf(stderr, "Error opening file \"%s\" for reading.\n", filename);
			exit(1);
		}
		firstrun = 0;
	}

	char *line = NULL;

	size_t linecap = 0;
	ssize_t linelen;
	linelen = getline(&line, &linecap, fp);

	if (linelen == -1) {
		fclose(fp);
		firstrun = 1;
		return NULL;
	}

	char *newline = strchr(line, '\n');
	if (newline) *newline='\0';
	return line;
}

//SPLIT STRING INTO TOKENS
void string_into_tokens(char *read_from) {

	char *new_line;
	char *filename = read_from;
	char *new_word;
	float score_temp;

	new_line = read_next_line(filename); //call read_next_line

	while (new_line != NULL) {
		new_word = strtok(new_line," 	"); //split sentence into words

		if (new_word == NULL) {  //empty sentence..end of reviews
			break;
		}

		score_temp = strtof(new_word, NULL); //first word is a float -> review score

		new_word = strtok(NULL," 	"); //first word of review

		while (new_word != NULL) {
			int i=0;

			while (new_word[i] != '\0') {   //turn all capital letters into lowercase
				if ( isupper(new_word[i]) ) {
					new_word[i]=tolower(new_word[i]);
				}
				i++;
			}

			insert_hashTable(new_word, score_temp); //call insert_hashTable

			new_word = strtok (NULL, " 	");
		}

		new_line = read_next_line(filename); //call read_next_line
	}
}

//HASH FUNCTION
unsigned long hash(char *word) {

	unsigned long hash = 5381;
	int c;

	while ((c = *word++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

//ALLOCATES MEMORY FOR WORDS AND ADDS WORDS INTO HASH TABLE
void add_nodeT(nodeT *head, char *word, float score, unsigned long position) {

	nodeT *new_nodeT;
	nodeT *curr; // temporary helping pointer

	for(curr=head; (curr!=NULL) && ((strcmp(curr->word,word)) != 0 ); curr=curr->next); // word search

	if (curr == NULL) { //not found

		new_nodeT = malloc(sizeof(nodeT));
		if (!new_nodeT) {
			printf("Error allocating memory.\n");
			exit(0);
		}

		new_nodeT->word = malloc((strlen(word)+1)*sizeof(char));
		if (!new_nodeT->word) {
			printf("Error allocating memory.\n");
			exit(0);
		}

		strcpy(new_nodeT->word, word);

		new_nodeT->next = hash_head->table[position]; //add new node to the list
		hash_head->table[position] = new_nodeT;

		hash_head->table[position]->num_app = 1;
		hash_head->table[position]->score = score;
		hash_head->num_entries ++;

		int load_factor = hash_head->num_entries / hash_head->size; //check load after inserting word in list

		if (load_factor > 3) {
			rehashing();
		}
	}
	else { //word found
		curr->num_app = curr->num_app + 1;
		curr->score = curr->score + score;
	}
}

//DECIDES THE PROPER POSITION FOR A NODE TO BE PUT INTO THE HASH TABLE via HASH FUNCTION
void insert_hashTable(char *word, float score) {

	unsigned long hash_value;
	unsigned long position;

	hash_value = hash(word);

	position = hash_value % (hash_head->size);

	add_nodeT(hash_head->table[position], word, score, position);
}

//PRINTS A LIST OF NODES
void print_nodeT_list(nodeT *head) {

	nodeT *curr;

	for (curr=head; curr!=NULL; curr=curr->next) {
		printf("[ \"%s\" %d %.2f ]", curr->word, curr->num_app, curr->score);

		if (curr->next != NULL) {
			printf(", ");
		}
		else {
			printf("\n");
		}
	}
}

//DESTROYS A LIST OF NODES AND DEALLOCATES MEMORY
void destroy_nodeT_list(nodeT *head, int position) {

	nodeT *curr, *temp;

	for (curr = head; curr != NULL;) {
		temp = curr->next;
		free(curr->word);
		free(curr);
		curr=temp;
	}
	hash_head->table[position] = NULL;
}

//EXTRACTS A NODE FROM A LIST OF NODES (DOESN'T FREE FROM MEMORY)
nodeT *extract_nodeT(nodeT *head, int i) {

	nodeT *extract;

	extract = head;
	hash_head->table[i] = hash_head->table[i]->next;
	extract->next = NULL;

	return(extract);
}

//PRINTS THE HASH TABLE
void print_hashTable() {

	int i;

	printf("\n");

	for (i=0; i<hash_head->size; i++) {

		if (hash_head->table[i] != NULL) {
			printf("%4d: ",i);
			print_nodeT_list(hash_head->table[i]);
		}
	}
	printf("\n");
}

//DESTROYS THE HASH TABLE AND FREES MEMORY
void destroy_hashT() {

	int i;

	for (i=0; i<hash_head->size; i++) {
		destroy_nodeT_list(hash_head->table[i], i);
	}

	for (i=0; i<hash_head->size; i++) {
		free(hash_head->table[i]);
	}

	free (hash_head);
}

//SEARCHES RECURSIVELY FOR THE BEST SCORE IN A LIST OF NODES
nodeT *best_score(nodeT *head) {

	float max; 
	nodeT *curr; 
	curr = head;

	if (curr == NULL) { //empty list
		return(NULL);
	}

	if (curr->next == NULL) { //end of list, begin of recursion
		return(curr);
	}
	else { //list continues
		max = curr->score / curr->num_app; //calculate temporary max
		curr = best_score(curr->next);
	}

	if ( (curr->score / curr->num_app) > max ) { //if max greater keep it
		max = curr->score / curr->num_app;
		return(curr);
	}
	else { //otherwise return the head of max
		return(head);
	}
}

//SEARCHES RECURSIVELY FOR THE BEST SCORE OF THE WHOLE HASH TABLE
nodeT *find_best(nodeT *head, int position) { //head = best_head->table[i]

	float max;

	hash_head->table[position] = head;

	if (position == (hash_head->size-1) ) {   //end of list, begin of recursion
		return(hash_head->table[position]);
	}
	else {

		if (hash_head->table[position] != NULL) {
			max = hash_head->table[position]->score / hash_head->table[position]->num_app;
		}
		else {
			max = 0;   //if null, it must find during recursion max == NULL
		}
		hash_head->table[position] = find_best(hash_head->table[position+1], position+1);

	}

	if (hash_head->table[position] != NULL) { //non-empty list

		if ( (hash_head->table[position]->score / hash_head->table[position]->num_app) > max ) { //if greater keep it
			max = hash_head->table[position]->score / hash_head->table[position]->num_app;
			return(hash_head->table[position]);
		}
		else {
			return(head); //otherwise return the head of max
		}
	}
	else {
		return(head); //empty list, keeps current head
	}
}

//REHASHING FUNCTION
void rehashing() {

	hashT *new_hashT;

	//allocate memory for new_hashT
	new_hashT = malloc(sizeof(hashT));
	if (!new_hashT) {
		printf("Error allocating memory.\n");
		exit(0);
	}

	int new_size;
	new_size = hash_head->size*2;  //double size
	new_hashT->size = new_size;

	new_hashT->num_entries = hash_head->num_entries;

	//allocate memory for table[i] of new_hashT
	new_hashT->table = malloc(sizeof(nodeT)*new_hashT->size);
	if (!new_hashT->table){
		printf("Error allocating memory.\n");
		exit(0);
	}

	int i;
	for (i=0; i<new_hashT->size; i++) {
		new_hashT->table[i] = NULL;
	}

	nodeT *extracted_nodeT;
	hashT *recover;

	for (i=0; i<hash_head->size; i++) {

		while (hash_head->table[i] != NULL) {

			extracted_nodeT = extract_nodeT(hash_head->table[i], i);

			recover = hash_head; //keeping hash head temporary, in order to have global reach
			hash_head = new_hashT;

			transfer_node(extracted_nodeT->word, extracted_nodeT); //call transfer_node

			hash_head = recover;
		}
	}

	free(hash_head->table);
	free(hash_head);

	hash_head = new_hashT; //point hash head to new hash table

}

//TRANSFERS A NODE FROM THE OLD HASH TABLE TO THE NEW ONE
void transfer_node(char *word, nodeT *extracted_nodeT) {

	unsigned long hash_value;
	unsigned long position;

	hash_value = hash(word);

	position = hash_value % (hash_head->size);

	extracted_nodeT->next = hash_head->table[position]; //connect new node to list
	hash_head->table[position] = extracted_nodeT;
}

//READ REVIEWS FROM KEYBOARD
void read_reviews() {

	char *input_line;
	char *new_word;
	int num_words = 0;
	float total_score = 0;
	float review_score;

	printf ("\nEnter review or DONE to finish:\n");

	input_line = malloc(sizeof(char)*1024);
	scanf("%[^\n]s", input_line);

	while (input_line != NULL) {

		num_words = 0;
		total_score = 0;

		new_word = strtok(input_line," ");

		if (strcmp(new_word, "DONE") == 0) {
			free(input_line);
			break;
		}

		while (new_word != NULL) {

			int i=0;
			while (new_word[i] != '\0') {
				if ( isupper(new_word[i]) ) {    //capital letters to lowercase
					new_word[i]=tolower(new_word[i]);
				}
				i++;
			}

			nodeT *curr;
			unsigned long hash_value;
			unsigned long position;
			float avg_score;

			hash_value = hash(new_word);

			position = hash_value % (hash_head->size);

			for (curr=hash_head->table[position]; (curr!=NULL) && ((strcmp(curr->word,new_word)) != 0 ); curr=curr->next); //search word

			if (curr != NULL) {   //word found
				num_words++;
				avg_score = curr->score / curr->num_app;
				total_score = total_score + avg_score;
			}

			new_word = strtok (NULL, " ");
		}

		if (num_words == 0) {
			printf("Sorry, there is no score for this review!\n");
		}
		else {
			review_score = total_score/num_words; //calculate review score and print the result
			printf("Review score: %.4lf\n", review_score);
			printf ("This review is ");

			if (review_score < 2) {
				printf ("negative.\n");
			}
			else if (review_score == 2) {
				printf ("neutral.\n");
			}
			else {
				printf ("positive.\n");
			}
		}

		free(input_line);
		printf("\nEnter review or DONE to finish:\n");
		scanf("\n");
		input_line = malloc(sizeof(char)*1024);
		scanf("%[^\n]s", input_line);

	}
}

//MAIN FUNCTION
int main(int argc, char *argv[]) {

	char *pargument = "-p";  //argument for printing hash table
	int i=0;

	if (argc > 3) { // if arguments > 3 end
		printf ("Incorrect number of parameters\n");
		return(0);
	}

	else if (argc == 3) { // if 3 search for -p argument

		while (argv[i] != NULL) {

			if (strcmp(argv[i], pargument) == 0) { // read -p

				char *read_from = argv[1]; // read from 2nd argument file name

				init_hash_table();

				string_into_tokens(read_from);

				hashT *best_head; //pointer type hashT for storing, every new table[i], the best word of every list

				best_head = malloc(sizeof(hashT));
				if (!best_head) {
					printf("Error allocating memory.\n");
					exit(0);
				}

				best_head->size = hash_head->size;

				best_head->table = malloc(sizeof(nodeT)*hash_head->size);
				if (!best_head->table) {
					printf("Error allocating memory.\n");
					exit(0);
				}

				int i;
				for (i=0; i<best_head->size; i++) {
					best_head->table[i] = NULL;
				}

				for (i=0; i<hash_head->size; i++) {
					best_head->table[i] = best_score(hash_head->table[i]); //call best_score function, store the best word
				}

				hashT *temp;
				temp = hash_head;
				hash_head = best_head; //global reach of best_head table

				nodeT *final_best;  //store result of find_best 

				final_best = malloc(sizeof(nodeT));

				if (!final_best) {
					printf("Error allocating memory.\n");
					exit(0);
				}

				i=0;
				final_best = find_best(hash_head->table[i], i); //the best word

				if (final_best != NULL) {
					printf("The most positive word is \"%s\" with a score of %.3f\n", final_best->word, final_best->score);
				}

				hash_head = temp;

				print_hashTable();

				read_reviews(); //database ready, ready for new input

				destroy_hashT();

				free(best_head->table);

				free(best_head);

				return(0);
			}
			i++;
		}

		printf ("Incorrect number of parameters\n"); // not -p in arguments list
		return(0);
	}
	else { //less than 3 arguments

		char *read_from = argv[1]; // read from 2nd argument

		init_hash_table();

		string_into_tokens(read_from);

		hashT *best_head;

		best_head = malloc(sizeof(hashT));
		if (!hash_head) {
			printf("Error allocating memory.\n");
			exit(0);
		}

		best_head->size = hash_head->size;

		best_head->table = malloc(sizeof(nodeT)*hash_head->size);
		if (!best_head->table) {
			printf("Error allocating memory.\n");
			exit(0);
		}

		int i;

		for (i=0; i<best_head->size; i++) {
			best_head->table[i] = NULL;
		}

		for (i=0; i<hash_head->size; i++) {
			best_head->table[i] = best_score(hash_head->table[i]);
		}

		hashT *temp;
		temp = hash_head;
		hash_head = best_head;

		nodeT *final_best;

		final_best = malloc(sizeof(nodeT));
		if (!final_best) {
			printf("Error allocating memory.\n");
			exit(0);
		}

		i=0;
		final_best = find_best(hash_head->table[i], i);

		if (final_best != NULL) {
			printf("The most positive word is \"%s\" with a score of %.3f\n", final_best->word, final_best->score);
		}

		hash_head = temp;

		read_reviews();

		destroy_hashT();

		free(best_head->table);

		free(best_head);

		return(0);
	}
}
