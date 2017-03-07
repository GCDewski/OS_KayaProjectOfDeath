/*****************************  PCB.c  *************************************** 
 * This module manages the queue of ProBlks by implementing:
 * 
 * The allocation and deallocation of ProcBlks
 * The maintenance of queues of ProcBlks
 * The maintenance of trees of ProcBlks 
 * 
 * This queue is a double circularly linked list handled w/ a tail pointer
 * 
 ****************************************************************************/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"

/****************Module global variables****************/

HIDDEN pcb_t *pcbList_tp; /*list tail pointer*/


 
/*****************Process Queue Maintenance*****************/

/* This method is used to initialize a variable to be tail pointer to a
process queue.
Return a pointer to the tail of an empty process queue; i.e. NULL. */
pcb_t *mkEmptyProcQ (){
	return(NULL);
}

/*Return true if tp is empty otherwise return false*/
int emptyProcQ (pcb_t *tp){
	return (tp == NULL); 
}

/* Insert the ProcBlk pointed to by p into the process queue whose
tail-pointer is pointed to by tp. Note the double indirection through
tp to allow for the possible updating of the tail pointer as well. */
void insertProcQ (pcb_t **tp, pcb_t *p) {

	/* if the procq is empty*/
	if(emptyProcQ((*tp))){
		/* the next/previous nodes for the current node, is also the current node*/
		p->p_next = p; 
		p->p_previous = p;
		
		/*the first node is also the tail pointer*/
		(*tp) = p;
	}
	/* if procq has at least one PCB */
	else{
		
		/*initialize pointers for the new node to be inserted */
		p->p_next = (*tp)->p_next; /* the new nodes next value is set to the first element*/
		p->p_next->p_previous = p; /* the previous value of the first node is set to the new node*/
		p->p_previous = (*tp);
		/* initialize the new tail pointer*/
		(*tp)->p_next = p;
		(*tp) = p;
	} 

}

/*Remove the head element from process queue and return a pointer to that removed element.*/

pcb_t *removeProcQ(pcb_t **tp) { 
	 
	 /*pcb_t *headPCB; temp var for headNode*/
	 
	/* if the procQ is empty */
	if(emptyProcQ(*tp)){
		
		return(NULL);
	
	}
	/*if the ProcQ only has 1 node*/
	else if((*tp)->p_next == *tp){
		pcb_t *headPCB = *tp;
		*tp = mkEmptyProcQ(); /*update tp to be removed*/
		
		return(headPCB);
		
	}
	/*if the ProcQ has 2+ nodes*/
	else{
		
		pcb_t *headPCB = (*tp)->p_next; /*set value of node going to be removed and returned.*/
		(((*tp)->p_next)->p_next)->p_previous = (*tp); /*set previous of new head to be the tail pointer*/
		(*tp)->p_next = (((*tp)->p_next)->p_next); /*set tp's next value as the new head of the list. Question: Can I use headPCB to see*/
		
		
		return(headPCB);

	}
 }
 

 
/*Remove the ProcBlk pointed to by p from the process queue. Update queue based on p's location in queue (ie fix next and previous stuff)*/
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
	
	pcb_t *removedNode; /*temp var for node being removed*/
	pcb_t *loopNode; /*temp var for loopNode*/
	
	/*if the procq is empty*/
	if(emptyProcQ(*tp)){	
		return(NULL);
	}
	
	/*if procq has only 1 node*/
	else if ((*tp)->p_next == *tp){
		if(*tp == p){
			removedNode = *tp;
			*tp = mkEmptyProcQ();
			return removedNode;
		}
		else{
			return(NULL);
		}
	}
	
	/*if procq has 2+ nodes*/
	else{
		
		/*If p is the tail pointer*/
		if(*tp == p){
			
			removedNode = (*tp); /*set value of node going to be removed and returned.*/
			(*tp)->p_previous->p_next = (*tp)->p_next;
			(*tp)->p_next->p_previous = (*tp)->p_previous;
			*tp = (*tp)->p_previous;
			
			return(removedNode);
		}
		
		/*Else p is the middle*/
		else{
			
			loopNode = ((*tp)->p_next);
			
			while(loopNode != *tp)
			{
				if(p == loopNode && loopNode == (*tp)->p_next)
				{
					return removeProcQ(tp);
				}
				
				else if(loopNode == p)
				{
					
					(p->p_next)->p_previous = (p->p_previous);
					(p->p_previous)->p_next = (p->p_next);
					
					return p;
				}
				
				else
				{		
					loopNode = loopNode->p_next;
				}
			}
			
			return(NULL);			
			
		}
	}
 
}
 /*Return first ProcBlk from process queue.*/
pcb_t *headProcQ(pcb_t *tp){
 
	if(!emptyProcQ(tp))
	{
		return (tp->p_next);
	}
	
	return  NULL;
 
}


/*****************Allocation & Deallocation of ProBlk's*****************/

/*Insert the element pointed to by p onto the pcbFree list*/
void freePcb(pcb_t *p){
	insertProcQ(&pcbList_tp, p);

}

 
/*Initialize the pcbFree list to contain all the elements of the static array of MAXPROC ProbBlk's. This is called only once */
void initPcbs() {

	static pcb_t procTable[MAXPROC];
	int i;
	i = 0;
	pcbList_tp = mkEmptyProcQ();
	while (i < MAXPROC) {
		freePcb(&procTable[i]);
		i = i+1;
	}
 
}

 /*Return Null if PCBFree list is empty. Otherwise, remove an element 
  from PCBfree list, provide initial values for all fields (Null/0) and 
  return a pointer to removed element.
*/
pcb_t *allocPcb(){
	pcb_t *removedElement;
	removedElement = removeProcQ(&pcbList_tp);
	
	if(removedElement != NULL){
		removedElement->p_next = NULL;
		removedElement->p_previous = NULL;
		removedElement->p_prnt = NULL;
		removedElement->p_child = NULL;
		removedElement->p_nxt_sib = NULL;
		removedElement->p_prev_sib = NULL;
		removedElement->p_semAdd = NULL;
		
		/*For Phase 2*/
		removedElement->p_types[0].oldState = NULL;
		removedElement->p_types[0].newState = NULL;
		removedElement->p_types[1].oldState = NULL;
		removedElement->p_types[1].newState = NULL;
		removedElement->p_types[2].oldState = NULL;
		removedElement->p_types[2].newState = NULL;
		
		removedElement->p_CPUTime = 0;
	}
	return(removedElement);
}


/*****************Process Tree Maintenance*****************/

/*Return T if the ProcBlk pointed to by p has no children. Return F otherwise.*/
 int emptyChild(pcb_t *p){
  
	 if(p->p_child == NULL){
		 return 1;
	 } else {
		 return 0;
	 }
 
}

/*Make the ProcBlk pointed to by p a child of the ProcBlk pointed to by prnt*/
void insertChild(pcb_t *prnt, pcb_t *p){
	
	/*if parent doesn't have any children, add p as first child*/
	if (emptyChild(prnt)) {
		prnt->p_child = p; /*set parent's child*/
		p->p_prnt = prnt; /*set child's parent*/
		p->p_prev_sib = NULL; /*child doesn't have siblings */
		p->p_nxt_sib = NULL;
		
	/*parent already had children, so change child of parent and set siblings*/	
	} else {
		
		p->p_nxt_sib = (prnt->p_child); /*set p's next sibling as parent's old child*/
		p->p_prev_sib = NULL; /*set previous sibling to NULL since it is first*/
		(prnt->p_child)->p_prev_sib = p; /*set old child's previous sibling*/ 
		
		p->p_prnt = prnt; /*set child's parent*/
		prnt->p_child = p; /*set new first child p*/
		
	}
}

/*Make the first child of the ProBlk pointed to by p no longer a child of
 *  p. Return NULL if there were no children of p. Otherwise return 
 * removed child */
pcb_t *removeChild(pcb_t *p){
  
  /*check if p doesn't have any children*/
  if (emptyChild(p)) {
	return NULL;
	
  /*if p only has one child, update parent's child to be null*/
  } else if ((p->p_child)->p_nxt_sib == NULL && (p->p_child)->p_prev_sib == NULL) {
	pcb_t *temp; /*temp var holds child being removed*/
	temp = p->p_child;
	
	p->p_child = NULL;
	
	return temp;
	
	/*if p has multiple children, make next sibling the new child for parent new child's previous is NULL*/
  } else {
	  pcb_t *temp;
	  temp = p->p_child;
	  
	  p->p_child = (temp)->p_nxt_sib; /*set new child for parent node as the next sibling of old child*/
	  (p->p_child)->p_prev_sib = NULL; /*set new child's previous sibling as null*/
	  
	  return temp;
  }
 
 
}
 
/*Make the child of the ProcBlk pointed to by p no longer a child of p. Return NULL is p has no parent. Otherwise return p.*/
pcb_t *outChild(pcb_t *p){
 
	 /*Check if p doesn't have a parent*/
	 if(p->p_prnt == NULL)
	 {
		 return NULL;
	 }
	 
	 /*If p is the only child of parent*/
	 else 
	 {
		 if (p->p_nxt_sib == NULL && p->p_prev_sib == NULL)
		 {
			 (p->p_prnt)->p_child = NULL; /*set parent's child to be NULL*/
			  p->p_prnt = NULL; /*set child to have no parent*/
			 
		 }
		 /*if p has siblings, so check where it is in the stack and update accordingly*/
		 else
		 {
			 /*is p the first child? If so, need to set parent's child to next sibling & that sib's previous is set to NULL*/
			 if(p->p_prev_sib == NULL) 
			 {
				(p->p_prnt)->p_child = p->p_nxt_sib; /*set new first child for parent*/
				(p->p_nxt_sib)->p_prev_sib = NULL; /*set new child's previous sibling as null*/
				
				p->p_prnt = NULL; /*set p to have no parent*/
				p->p_nxt_sib = NULL; /*set p to have no next sibling. Previous sib is already NULL*/
			
			 }
			 
			 /*is p the last child? If so, previous sibling needs to have next sibling set to NULL*/
			 else if (p->p_nxt_sib == NULL)
			 {
				(p->p_prev_sib)->p_nxt_sib = NULL;
				 
				p->p_prnt = NULL; /*set p to have no parent*/
				p->p_prev_sib = NULL; /*set p to have no previous sibling. Next sib is already NULL*/
				 
			 }
			 
			 /*else, just update siblings 'cause it's a middle child*/
			 else
			 {
				 (p->p_nxt_sib)->p_prev_sib = (p->p_prev_sib); /*set p's next sibling to go to p's previous sibling*/
				 (p->p_prev_sib)->p_nxt_sib = (p->p_nxt_sib); /*set p's previous sibling to go to p's next sibling*/
				 
				 p->p_prnt = NULL;
				 p->p_prev_sib = NULL;
				 p->p_nxt_sib = NULL;
				 
			 }
		 }
		 
		 return p;
	 }
 
}
