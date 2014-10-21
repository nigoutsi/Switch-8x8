 /* 
	Gkoutsidis Nikolaos
	AEM: 335 
*/

/* Switch 8x8 Simulation*/

/* Header files */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables */
int id_number = 100; /* ID unique for each packet */
double load = 0.0; /* Maximum average load of system */
double delay_sum = 0; /* Sum of delay for calculation of average delay time for all system */
int num_serv_pack = 0; /* Number of served packets for calculation of average delay time */
int num_all_packets = 0; /* Number of all received packets */

/* Files for output data*/
FILE *fp, *fp1, *fp2, *fp3, *fp4, *fp5;

/* Simply linked lists */
typedef struct packet {
	int id;
	int queue_num; /* Number of input queue */
	int dest; /* Destination output number */
	int slot_arr; /* Number of arrived slot */
	struct packet * next; /* Next pointer */
} PacketNode;

/* Type queue */
typedef struct {
	PacketNode * first;
	PacketNode * last;
} queue;

/* Empty queue */
const queue queueEmpty = { NULL, NULL };

/* Insert packet */
void queueInsert(queue * qp, int queue_number, int destination_queue, int slot_arrived)
{
	PacketNode * n = (PacketNode *)malloc(sizeof(PacketNode));

	/* Check if malloc succeeded */
	if (n == NULL) {
		fprintf(fp3, "Out of memory\n");
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}

	/* Copy the data */
	n->id = id_number++;
	n->queue_num = queue_number;
	n->dest = destination_queue;
	n->slot_arr = slot_arrived;
	n->next = NULL;

	/* If the queue was empty, just add this one element */
	if (qp->last == NULL)
		qp->first = qp->last = n;

	/* Otherwise, add the new element at the end */
	else {
		qp->last->next = n;
		qp->last = n;
	}
	num_all_packets++;
}

/* Remove packet and return the arrived slot of packet to calculate the delay */
int queueRemove(queue * qp, int slot_servred)
{
	PacketNode * n = qp->first;
	int s_a; /* Slot arrived */

	/* Error, if the queue was empty */
	if (qp->first == NULL) {
		// Nothing to remove from an empty queue 
		return -1;
	}

	/* Sum of Delay for all system */
	delay_sum = delay_sum + (slot_servred - (qp->first->slot_arr));

	/* Print results to file*/
	fprintf(fp, "%d\t\t%d\t\t%d\t\t%d\t\t%d\t%d\n", n->id, n->queue_num, n->dest, n->slot_arr, slot_servred, (slot_servred - (qp->first->slot_arr)));

	/* Remove and free the first element */
	s_a = n->slot_arr;
	qp->first = qp->first->next;
	free(n);

	/* If the queue is now empty, set the 'last' pointer too */
	if (qp->first == NULL)
		qp->last = NULL;

	/* Increase the number of served packet */
	num_serv_pack = num_serv_pack + 1;

	return s_a;
}

void print_queue(queue * qp){
	PacketNode * n = qp->first;
	if (n == NULL)
		fprintf(fp4, "Empty queue.\n");
	while (n != NULL){
		fprintf(fp4, "id = %d, queue = %d, dest = %d. ", n->id, n->queue_num, n->dest);
		n = n->next;
	}
	fprintf(fp4, "\n\n");
}

int main(int argc, char *argv[]){
	/* Variables */
	int slot = 0; /* Slot time */ 
	int i, j, arr_slot, max, pos_i, pos_j, sum_queues, temp = 0, temp2 = 0; /*temp , temp2 temporaty variables for packet genarator */
	int in_out[8][8]; /* in_out -> steady matrix for input-output pairs*/
	double delay_in_out[8][8]; /* delay_in_out -> sum of delay time for each pair input-output */
	double avg_delay_in_out[8][8]; /* avg_delay_in_out -> average of delay time for each pair input-output */
	int serv_in_out[8][8]; /* numbers of served packets for each pair input-output */
	int max_pos[8][8]; /* position for each maximum queue, the permutation matrix */
	int size_q[8][8]; /* size of each queue */
	int mat_temp[8][8]; /* temporary matrix */
	double prob[8]; double temp_sum = 0; double a, val, avg; /* prob -> probability for each input */
	queue mat[8][8]; /* the pointer matrix which points to each queue */

	/* Files */
	fp = fopen("results.txt", "w");
	fp1 = fopen("permutation_matrices.txt", "w");
	fp2 = fopen("queues_size.txt", "w");
	fp3 = fopen("errors.txt", "w");
	fp4 = fopen("queues_remain.txt", "w");
	fp5 = fopen("final_results.txt", "w");
	fprintf(fp, "ID\tQueue Number\tDest Queue\tSlot Arrived\tSlot Served\tDelay\n");

	/* Scan from console the maximum average load of system */
	printf("\nGive the avegare maximum load (e.g. 0.1-1.8):  ");
	scanf("%lf", &load);
	while ((load<0.0) || (load>1.8)){
		printf("\nError\n");
		printf("\nGive the avegare maximum load (0.1-1.8):  ");
		scanf("%lf", &load);
	}

	srand(time(NULL)); // randomize seed for random

	/* Initialized all matrices */
	for (i = 0; i < 8; i++){
		for (j = 0; j < 8; j++){
			mat[i][j] = queueEmpty;
			max_pos[i][j] = 0;
			size_q[i][j] = 0;
			mat_temp[i][j] = 0;
			delay_in_out[i][j] = 0;
			serv_in_out[i][j] = 0;
			avg_delay_in_out[i][j] = 0;
		}
	}

	/* Calculate the probability of each queue */
	for (i = 0; i < 8; i++){
		prob[i] = ((i + 1) % 4);
		temp_sum = temp_sum + 3 * prob[i];
	}
	a = (8 * load) / temp_sum; /* Calulate α and print it*/
	printf("a = %f \n", a);
	printf("\nArray with probabilities for inputs:\n"); /* Calculate and print the probabilities for each input */
	for (i = 0; i < 8; i++){
		prob[i] = prob[i] * a;
		if (prob[i]>1){ /* if the prob is greater than 1 */ 
			prob[i] = 1;
			printf("\nThe system is unstable...\n"); /* Here the system is unstable */
		}
		printf("%f ", prob[i]);
	}
	printf("\n");

	/* Array for pairs input-output with distribution 0.5 fixed */
	printf("\nArray Input/Output:\n");
	for (i = 0; i < 8; i++){
		for (j = 0; j < 8; j++){
			val = (double)rand() / RAND_MAX;
			if (val < 0.5)
				in_out[i][j] = 0;
			else
				in_out[i][j] = 1;
			printf("%d ", in_out[i][j]);
		}
		printf("\n");
	}

	/* Simulate for 5000 slots */
	while (slot < 5000){

		/* Choose the pairs with max-weigth matching and Serve the packets */
		if (slot != 0){ /* for the slot=0 no serve packets */
			fprintf(fp2, "\n Queues Size in %d slot: \n", slot); /* print the sizes of queues and copy them in a temp mat*/
			for (i = 0; i < 8; i++){
				for (j = 0; j < 8; j++){
					fprintf(fp2, " %d ", size_q[i][j]);
					mat_temp[i][j] = size_q[i][j];
				}
				fprintf(fp2, "\n");
			}
			
			/* Find the sum of packets from temporary matrix */
			sum_queues = 0;
			for (i = 0; i < 8; i++){
				for (j = 0; j < 8; j++){
					sum_queues = sum_queues + mat_temp[i][j];
				}
			}
			/* Find the max positions and note the position with 1. Calulate the permutation matrix */
			while (sum_queues != 0){ /* if exists packets for service in temp mat */
				max = 0; pos_i = -1; pos_j = -1;
				for (i = 0; i < 8; i++){
					for (j = 0; j < 8; j++){
						if ((mat_temp[i][j]>max) && (mat_temp[i][j] != 0)){ /* Find the max and position i,j of it */
							max = mat_temp[i][j];
							pos_i = i;
							pos_j = j;
						}
					}
				}
				if ((pos_i != -1) && (pos_j != -1)){
					if (max != 0){
						max_pos[pos_i][pos_j] = 1; /* Note with 1 the posotion of max in permutation mat */
					}
					/* Zero the line and the column of the max */
					for (i = 0; i < 8; i++){
						mat_temp[pos_i][i] = 0;
						mat_temp[i][pos_j] = 0;
					}
				}
				/* zero the temp and find the sum of packets for service */
				sum_queues = 0;
				for (i = 0; i < 8; i++){
					for (j = 0; j < 8; j++){
						sum_queues = sum_queues + mat_temp[i][j];
					}
				}
			}

			/* Print the permutation mat */
			fprintf(fp1, "\n Permutation matrix for %d slot : \n", slot);
			for (i = 0; i < 8; i++){
				for (j = 0; j < 8; j++){
					fprintf(fp1, " %d ", max_pos[i][j]);
				}
				fprintf(fp1, "\n");
			}

			/* Serve the packets */
			for (i = 0; i < 8; i++){
				for (j = 0; j < 8; j++){
					if ((max_pos[i][j] == 1) && (&mat[i][j] != NULL)){ /* From permutation mat and if the queue has packets */
						arr_slot = queueRemove(&mat[i][j], slot); /* serve the packet */
						serv_in_out[i][j] = serv_in_out[i][j] + 1; /* Increase the served packets */
						if (arr_slot == -1) /* if there is no packet to serve */
							fprintf(fp3, "No packet to remove in queue %d to dest %d.\n", i, j);
						else{
							delay_in_out[i][j] = delay_in_out[i][j] + (slot - arr_slot); /* add the delay time */
							size_q[i][j] = size_q[i][j] - 1; /* Decrease the size of the queue */
						}
					}
				}
			}
		}

		/* Born of packets in each queue and choose the destination */
		for (i = 0; i < 8; i++){
			val = (double)rand() / RAND_MAX; /* Find a random value*/
			if (val < prob[i]){ /* if the probability of i input is greater than the random value */
				/*Born 3 packets*/
				temp = 0;
				for (j = 0; j < 8; j++){
					if (in_out[i][j] == 1)
						temp++; /* Find the number of outputs */
				}
				if (temp != 0){ /* if there is outputs */
					temp2 = (rand() % temp) + 1; /* Choose the destination-output queue */
					for (j = 0; j < 8; j++){
						if (in_out[i][j] == 1){
							temp2--; /* Find the exactly output which calculated from random */
							if (temp2 == 0){
								/* Put 3 packets in i,j queue*/
								queueInsert(&mat[i][j], i, j, slot);
								queueInsert(&mat[i][j], i, j, slot);
								queueInsert(&mat[i][j], i, j, slot);
								size_q[i][j] = size_q[i][j] + 3; /* Increase the size of i,j queue */
							}
						}
					}
				}
			}
		}

		/* zero the permutation mat for the next slot */
		for (i = 0; i < 8; i++){
			for (j = 0; j < 8; j++){
				max_pos[i][j] = 0;
			}
		}
		/* next slot Slot + 1 */
		slot++;
	}

	/* Print the remainings packets after the simulation which not served */
	for (i = 0; i < 8; i++){
		for (j = 0; j < 8; j++){
			fprintf(fp4, "For Queue %d with Dest %d  has %d packets: \n", i, j, size_q[i][j]);
			print_queue(&mat[i][j]);
		}
	}

	avg = (delay_sum / num_serv_pack); /* average delay for all sysytem */
	printf("\nAverage Delay Time for %d served packets \nis %f slots \nfor %d slots.\n", num_serv_pack, avg, slot);
	printf("\nSee the results in txt files.\n");

	/* Print the results in files */
	fprintf(fp5, "Number of Received Packets: %d\n", num_all_packets);
	fprintf(fp5, "Number of Served Packets: %d\n", num_serv_pack);
	fprintf(fp5, "Slot Time: %d\n", slot);
	fprintf(fp5, "Average Maximum Load: %f%% with α = %f \n", load*100,a);
	fprintf(fp5, "Total Average Delay Service Time (Slots): %f\n\n", avg);

	/* Calculate and Print the average delay time for each queue */
	fprintf(fp5, "Average Delay Service Time (Slots) for each pair Input/Output is:\n\n");
	for (i = 0; i < 8; i++){
		for (j = 0; j < 8; j++){
			if (serv_in_out[i][j] != 0){
				avg_delay_in_out[i][j] = delay_in_out[i][j] / serv_in_out[i][j]; /* Calculate the average delay time for i,j queue */
				fprintf(fp5, "Average Delay Time (Slots) for Input %d and Output %d is %f \n", i, j ,avg_delay_in_out[i][j]);
			}
			else /* if the i,j queue was empty all the time */
			{
				fprintf(fp5, "Average Delay Time (Slots) for Input %d and Output %d is 0 because has not packets to serve.\n", i, j);
			}
		}
	}

	/* Average Delay for all queues from the each pair */
	temp_sum = 0;
	for (i = 0; i < 8; i++){
		for (j = 0; j < 8; j++){
			temp_sum = temp_sum + avg_delay_in_out[i][j];
		}
	}
	fprintf(fp5, "\nAverage Delay Time (Slots) for all queues from the each pair is %f \n", temp_sum/64);

	return 0;
}
