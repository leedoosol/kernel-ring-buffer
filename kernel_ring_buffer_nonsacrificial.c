#include<linux/kernel.h>
#include<linux/slab.h>

// indicates the maximum size of ring buffer
int cbuff_max_size;

// flag to indicate whether the ring buffer is full or not 
int cbuff_full;

// data node, can be changed as per requirements
// default is (struct node){-1, -1}
struct node{
	int distance;
	long timestamp;
};

// node of the ring buffer
struct cbuff_node{
	struct cbuff_node *next_ptr;	// points to the next element in ring
	struct node *node_ptr;			// points to the data node
};

// ring buffer representation
struct cbuff{
	struct cbuff_node *head;		// points to the node after the latest written node
	struct cbuff_node *tail;		// points to the node after the latest read node
};

// APIs for ring buffer

struct cbuff *cbuff_init(int size);

int is_cbuff_full(struct cbuff *cb);

int is_cbuff_empty(struct cbuff *cb);

void cbuff_destroy(struct cbuff* cb);

void cbuff_put(struct cbuff *cb, struct node node);

struct node cbuff_get(struct cbuff *cb);

/* 
 * called to create a circular buffer
 * returns a pointer to the created circular buffer
 * @size - size of the circular buffer required
 */
struct cbuff *cbuff_init(int size){
	
	int i;
	struct cbuff *cbuff;

	// global variables initialized
	cbuff_max_size = size;
	cbuff_full = 0;
	
	// ring buffer memory allocated
	cbuff = kmalloc(sizeof(struct cbuff), GFP_KERNEL);

	// head of ring buffer initialized
	cbuff->head = kmalloc(sizeof(struct cbuff_node), GFP_KERNEL);
	cbuff->head->node_ptr = kmalloc(sizeof(struct node), GFP_KERNEL);
	*cbuff->head->node_ptr = (struct node){-1, -1};

	// tail of ring buffer initialized
	cbuff->tail = cbuff->head;

	// remaining nodes of ring buffer initialized
	struct cbuff_node *temp = cbuff->head;
	for(i = 0; i < size; i++){
		if(i == size - 1){
			temp->next_ptr = cbuff->head;
		}else{
			temp->next_ptr = kmalloc(sizeof(struct cbuff_node), GFP_KERNEL);
			temp->next_ptr->node_ptr = kmalloc(sizeof(struct node), GFP_KERNEL);
			*temp->next_ptr->node_ptr = (struct node){-1, -1};
			temp = temp->next_ptr;
		}
	}

	return cbuff;
}

/*
 * called to destroy a circular buffer
 * @cb - circular buffer to be destroyed
 */
void cbuff_destroy(struct cbuff *cb){

	int i;

	// free all ring buffer nodes
	struct cbuff_node *temp1 = cb->head;
	struct cbuff_node *temp2 = cb->head->next_ptr;
	for(i = 0; i < cbuff_max_size; i++){
		kfree(temp1->node_ptr);		
		kfree(temp1);
		temp1 = temp2;
		temp2 = temp2->next_ptr;
	}
	cb->head = temp2;
	cb->tail = temp2;

	// free ring buffer
	kfree(cb);
}

/*
 * checks if a circular buffer is full
 * @cb - circular buffer to be checked
 * return 0 if false, 1 if true
 */
int is_cbuff_full(struct cbuff *cb){
	
	if((cb->tail == cb->head) && (cbuff_full == 1)){
		return 1;
	}else{
		return 0;
	}
}

/*
 * checks if a circular buffer is empty
 * @cb - circular buffer to be checked
 * return 0 if false, 1 if true
 */
int is_cbuff_empty(struct cbuff *cb){

	if((cb->tail == cb->head) && (cbuff_full == 0)){
		return 1;
	}else{
		return 0;
	}
}

/*
 * puts a node at the head position of circular buffer
 * @cb - circular buffer to be used
 * @node - node which needs to be inserted
 */
void cbuff_put(struct cbuff *cb, struct node node){

	*cb->head->node_ptr = node;

	cb->head = cb->head->next_ptr;

	if(cb->tail == cb->head && cbuff_full == 0){
		cbuff_full = 1;
		
	}else if(cbuff_full == 1){
		cb->tail = cb->tail->next_ptr;
	}
}

/*
 * returns a node from the tail position of circular buffer
 * @cb - circular buffer to be used
 */
struct node cbuff_get(struct cbuff *cb){
	
	struct node ret;
	
	if(cbuff_full == 1){
		cbuff_full = 0;
		ret = *cb->tail->node_ptr;
		*cb->tail->node_ptr = (struct node){-1, -1};
		cb->tail = cb->tail->next_ptr;
		return ret;
	}else if(is_cbuff_empty(cb)){
		return (struct node) {-1, -1};
	}else{
		ret = *cb->tail->node_ptr;
		*cb->tail->node_ptr = (struct node){-1, -1};
		cb->tail = cb->tail->next_ptr;
		return ret;
	}
}

/*
 * clears the value of nodes to default values
 * @cb - circular buffer to be cleared
 */
void cbuff_clear(struct cbuff *cb){
	if(is_cbuff_empty(cb)){
		return;
	}else{
		while(!is_cbuff_empty(cb)){
			*cb->tail->node_ptr = (struct node){-1, -1};
			cb->tail = cb->tail->next_ptr;
		}
	}
}

// to test the ring buffer
int main(void){
	int i;
	struct cbuff *cb;
	struct node n1;
	struct node n2;

	n1 = {2, 1};
	cb = cbuff_init(8);

	cbuff_put(cb, n1);
	n2 = cbuff_get(cb);
	// printk(KERN_INFO "%d %d\n", n1.distance, n1.timestamp);

	cbuff_clear(cb);
	cbuff_destroy(cb);	

	return 0;
}