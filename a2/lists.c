#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"

/* Add a group with name group_name to the group_list referred to by 
* group_list_ptr. The groups are ordered by the time that the group was 
* added to the list with new groups added to the end of the list.
*
* Returns 0 on success and -1 if a group with this name already exists.
*
* (I.e, allocate and initialize a Group struct, and insert it
* into the group_list. Note that the head of the group list might change
* which is why the first argument is a double pointer.) 
*/
int add_group(Group **group_list_ptr, const char *group_name) {
	Group *g;
	g = find_group(*group_list_ptr, group_name);

	if(g == NULL){
		Group* new_group;
		new_group = (Group*) malloc(sizeof(Group));
		if(new_group == NULL){
			exit(1);
		}

		new_group->name = NULL;
		new_group->users = NULL;
		new_group->xcts = NULL;
		new_group->next = NULL;

		new_group->name = (char *) malloc(sizeof(char) * (strlen(group_name) + 1));
		if(new_group->name == NULL){
			exit(1);
		}
		strcpy(new_group->name, group_name);

		if (*group_list_ptr == NULL){
			*group_list_ptr = new_group;
		}else{
			Group* g_node = *group_list_ptr;
			while (g_node->next != NULL){
				g_node->next = g_node;
			}
			g_node->next = new_group;
		}
		
		return 0;
	}else{
		return -1;
	}
}

/* Print to standard output the names of all groups in group_list, one name
*  per line. Output is in the same order as group_list.
*/
void list_groups(Group *group_list) {
	Group* g_node = group_list;
	while(g_node != NULL){
		printf("%s\n", g_node->name);
		g_node = g_node->next;
	}
}

/* Search the list of groups for a group with matching group_name
* If group_name is not found, return NULL, otherwise return a pointer to the 
* matching group list node.
*/
Group *find_group(Group *group_list, const char *group_name) {
	Group* g_node = group_list;
	while (g_node != NULL){
		if (strcmp(g_node->name, group_name) == 0){
			return g_node;
		}		
		g_node = g_node->next;
	}	

    return NULL;
}

/* Add a new user with the specified user name to the specified group. Return zero
* on success and -1 if the group already has a user with that name.
* (allocate and initialize a User data structure and insert it into the
* appropriate group list)
*/
int add_user(Group *group, const char *user_name) {
	User * user = find_prev_user(group, user_name);
	if (user == NULL){
		User* new_user;
		new_user = (User*) malloc(sizeof(User));
		if(new_user == NULL){
			exit(1);
		}
		new_user->name = (char *) malloc(sizeof(char) * (strlen(user_name) + 1));
		if(new_user->name == NULL){
			exit(1);
		}
		strcpy(new_user->name, user_name);
		new_user->balance = .0;
		//new_user->next = NULL;

		new_user->next = group->users;
		group->users = new_user;
		/*			
		if (group->users == NULL){
			group->users = new_user;
		}else{	
			
			User* u_user = group->users;
			while (u_user->next != NULL){
				u_user->next = u_user;
			}
			u_user->next = new_user;
		}
		*/
		return 0;
	}else{
		return -1;
	}
}

/* Remove the user with matching user and group name and
* remove all her transactions from the transaction list. 
* Return 0 on success, and -1 if no matching user exists.
* Remember to free memory no longer needed.
* (Wait on implementing the removal of the user's transactions until you 
* get to Part III below, when you will implement transactions.)
*/
int remove_user(Group *group, const char *user_name) {
	User * user = find_prev_user(group, user_name);
	if (user == NULL){
		return -1;
	}else{
		if (group->users == user && strcmp(user->name, user_name) == 0){
			group->users = user->next;
			free(user->name);
			free(user);
		}else{
			User* target_user = user->next;
			user->next = target_user->next;
			free(target_user->name);
			free(target_user);
		}
		remove_xct(group, user_name);
	}	

    return 0;
}

/* Print to standard output the names of all the users in group, one
* per line, and in the order that users are stored in the list, namely 
* lowest payer first.
*/
void list_users(Group *group) {
	User* u_user = group->users;
	while(u_user != NULL){
		printf("%s %lf\n", u_user->name, u_user->balance);
		u_user = u_user->next;
	}
}

/* Print to standard output the balance of the specified user. Return 0
* on success, or -1 if the user with the given name is not in the group.
*/
int user_balance(Group *group, const char *user_name) {
	User * user = find_prev_user(group, user_name);
	if (user == NULL){
		return -1;
	}else{
		if (strcmp(user->name, user_name) == 0){

		}else{
			user = user->next;
		}
		printf("%lf\n", user->balance);

		return 0;
	}
}

/* Print to standard output the name of the user who has paid the least 
* If there are several users with equal least amounts, all names are output. 
* Returns 0 on success, and -1 if the list of users is empty.
* (This should be easy, since your list is sorted by balance). 
*/
int under_paid(Group *group) {
	if (group->users == NULL){
		return -1;
	}else{
		double balance;
		User* u_user = group->users;
		balance = u_user->balance;
		do {
			printf("%s\n", u_user->name);
			u_user = u_user->next;
			if (u_user == NULL){
				break;
			}else{
				if (u_user->balance == balance){
				}else{
					break;
				}				
			}			
		} while (1);		

		return 0;
	}
}

/* Return a pointer to the user prior to the one in group with user_name. If 
* the matching user is the first in the list (i.e. there is no prior user in 
* the list), return a pointer to the matching user itself. If no matching user 
* exists, return NULL. 
*
* The reason for returning the prior user is that returning the matching user 
* itself does not allow us to change the user that occurs before the
* matching user, and some of the functions you will implement require that
* we be able to do this.
*/
User *find_prev_user(Group *group, const char *user_name) {
    if (group->users == NULL){
		return NULL;
	}else{
		User* p_user = group->users;
		if(strcmp(p_user->name, user_name) == 0){
			return p_user;
		}else{
			User* next_user;
			next_user = p_user->next;
			while(next_user != NULL){
				if(strcmp(next_user->name, user_name) == 0){
					return p_user;
				}else{
					p_user = next_user;
					next_user = next_user->next;
				}
			}
			return NULL;
		}
	}
}

/* Add the transaction represented by user_name and amount to the appropriate 
* transaction list, and update the balances of the corresponding user and group. 
* Note that updating a user's balance might require the user to be moved to a
* different position in the list to keep the list in sorted order. Returns 0 on
* success, and -1 if the specified user does not exist.
*/
int add_xct(Group *group, const char *user_name, double amount) {
	User * user = find_prev_user(group, user_name);
	if (user == NULL){
		return -1;
	}else{
		Xct * sct = (Xct*)malloc(sizeof(Xct));
		if(sct == NULL){
			exit(1);
		}
		sct->name = (char*)malloc(sizeof(char) * (strlen(user_name)+1));
		if (sct->name == NULL){
			exit(1);
		}
		
		strcpy(sct->name, user_name);
		sct->amount = amount;
		sct->next = NULL;

		sct->next = group->xcts;
		group->xcts = sct;

		if (group->users == user){
			User * c_user;
			User * next_user;
			User * next_next_user;

			c_user = user;
			c_user->balance += amount;

			next_user = c_user->next;
			if (next_user != NULL && next_user->balance < c_user->balance){
				group->users = next_user;

				next_next_user = next_user->next;
				while (next_next_user != NULL && next_next_user->balance < c_user->balance){
					next_user = next_next_user;
					next_next_user = next_user->next;
				}

				next_user->next = c_user;
				c_user->next = next_next_user;
			}
		}else{
			User * c_user;
			User * next_user;
			User * next_next_user;

			c_user = user->next;
			c_user->balance += amount;

			next_user = c_user->next;
			if (next_user != NULL && next_user->balance < c_user->balance){
				user->next = next_user;
				
				next_next_user = next_user->next;
				while (next_next_user != NULL && next_next_user->balance < c_user->balance){
					next_user = next_next_user;
					next_next_user = next_user->next;
				}
				
				next_user->next = c_user;
				c_user->next = next_next_user;
				
			}			
			
		}

		return 0;
	}
}

/* Print to standard output the num_xct most recent transactions for the 
* specified group (or fewer transactions if there are less than num_xct 
* transactions posted for this group). The output should have one line per 
* transaction that prints the name and the amount of the transaction. If 
* there are no transactions, this function will print nothing.
*/
void recent_xct(Group *group, long num_xct) {
	int i = 0;
	Xct* xct = group->xcts;
	while(xct != NULL && i < num_xct){
		printf("%s %lf\n", xct->name, xct->amount);
		++i;
		xct = xct->next;
	}
}

/* Remove all transactions that belong to the user_name from the group's 
* transaction list. This helper function should be called by remove_user. 
* If there are no transactions for this user, the function should do nothing.
* Remember to free memory no longer needed.
*/
void remove_xct(Group *group, const char *user_name) {
	Xct* next_xct;
	Xct* c_xct = group->xcts;

	while (c_xct != NULL && strcmp(c_xct->name, user_name) == 0){
		group->xcts = c_xct->next;
		free(c_xct->name);
		free(c_xct);
		c_xct = group->xcts;
	}
	
	if(c_xct != NULL){
		next_xct = c_xct->next;
		while(next_xct != NULL){
			if (strcmp(next_xct->name, user_name) == 0){
				c_xct->next = next_xct->next;
				free(next_xct->name);
				free(next_xct);
			}else{
				c_xct = next_xct;
			}
			next_xct = c_xct->next;
		}
	}
}
