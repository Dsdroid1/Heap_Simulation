#include<stdio.h>
#include<stdlib.h>
//Simulation of fibonacci heap by char array
#define HEAP_SIZE 13//Will be a large fibonacci number
#define MAX_FIB_POS 6//1,2,3,5,8,13
int Fib[MAX_FIB_POS];

void Generate_Fibo_Arr()
{
    int f=1,s=2;
    int i=0;
    while(i<MAX_FIB_POS)
    {
        Fib[i]=f;
        if(i+1<MAX_FIB_POS)
        {
            Fib[i+1]=s;
        }
        f=s+f;
        s=s+f;
        i=i+2;
    }
}

int get_index_of_fib(int f)
{
    int i=0;
    while(Fib[i]<f)
    {
        i++;
    }
    return i;
}

typedef struct Allot_tag
{
    char *start;
    struct Allot_tag *next;
    int size;
}Allot_list;

typedef struct Node
{
    char *start;//Starting address
    //int size;//No.of bytes
    struct Node *next;
}Heap_Node;

typedef struct Header_node_tag
{
    int size;//fibo number
    struct Node *next;
    struct Header_node_tag *down;//arranged in ascending order
}Free_list;

typedef struct heap_tag
{
    char heap_space[HEAP_SIZE];
}Heap;

void print_list_status(Free_list *flptr,Allot_list *alptr);
void Init(Heap H,Free_list **flptr,Allot_list **alptr)
{
    Heap_Node *h;
    h=(Heap_Node *)malloc(sizeof(Heap_Node));
    *flptr=(Free_list *)malloc(sizeof(Free_list));
    h->start=&H.heap_space[0];
    h->next=NULL;
    (*flptr)->size=HEAP_SIZE;
    (*flptr)->next=h;
    (*flptr)->down=NULL;
    *alptr=NULL;//nothing alloted yet
    //print_list_status(flptr,alptr);
}

void Divide(Free_list **pflptr,Free_list **pptr,Free_list **pprev,int fib_pos)//double ptrs required to change
{
    Free_list *node1,*node2,*flptr,*ptr,*prev,*tptr;
    Heap_Node *h1,*h2,*divnode,*tmp;
    flptr=*pflptr;
    ptr=*pptr;
    prev=*pprev;
    divnode=ptr->next;
    ptr->next=divnode->next;
    if(ptr->next==NULL)
    {
        if(prev!=NULL)
        {
            prev->down=ptr->down;
        }
        else
        {
            flptr=ptr->down;
        }
        free(ptr);
    }
   
    h1=(Heap_Node *)malloc(sizeof(Heap_Node));
    h2=(Heap_Node *)malloc(sizeof(Heap_Node));
    h1->start=divnode->start;
    h1->next=NULL;
    h2->start=divnode->start+Fib[fib_pos-1]*sizeof(char);
    h2->next=NULL;
    if(prev!=NULL)
    {
        if(prev->size==Fib[fib_pos-1])//Buddy size list exists
        {
            h1->next=prev->next;
            prev->next=h1;
            node1=prev;
        }
        else
        {
            node1=(Free_list *)malloc(sizeof(Free_list));
            node1->size=Fib[fib_pos-1];
            node1->next=h1;
            prev->down=node1;
        }
    }
    else
    {
        node1=(Free_list *)malloc(sizeof(Free_list));
        node1->size=Fib[fib_pos-1];
        node1->next=h1;
        //flptr=node1;
        node1->down=flptr;
        flptr=node1;
    }
    node2=(Free_list *)malloc(sizeof(Free_list));
    node2->size=Fib[fib_pos];
    node2->next=h2;
    //node1->down=node2;
    node2->down=node1->down;
    node1->down=node2;

    *pflptr=flptr;
    *pptr=ptr;
    *pprev=prev; 
}

//Will have dynamic free lists and an alloc list
//has to handle buddy of 1(divide doesnt)
char * Allot(int size,Heap H,Free_list **pflptr,Allot_list **palptr)
{
    //Get the closest two fibonacci numbers
    Free_list *ptr,*prev,*flptr;
    Allot_list *alnode,*alptr;
    Heap_Node *hptr;
    char *retptr=NULL;
    int allot,buddy;
    flptr=*pflptr;
    alptr=*palptr;
    int i=0,fib_pos;
    while(Fib[i]<size&&i<MAX_FIB_POS)
    {
        i++;
    }
    allot=Fib[i];
    buddy=Fib[i-1];
    ptr=flptr;
    prev=NULL;
    //Search for an existing free list of required size or a list with just greater size
    while(ptr->size<allot&&ptr->down!=NULL)
    {
        prev=ptr;
        ptr=ptr->down;
    }
    if(ptr->next==NULL&&ptr->size<allot)
    {
        //Error,cant give memory,contiguous memory not available
    }
    else
    {
        if(ptr->size==allot)
        {
           retptr=ptr->next->start;
           hptr=ptr->next;
           ptr->next=hptr->next;
           free(hptr);
           if(ptr->next==NULL)//This size of free space are now no longer available
           {
               if(prev!=NULL)
               {
                   prev->down=ptr->down;
               }
               else
               {
                   flptr=ptr->down;
               }
               free(ptr);
           }
           alnode=(Allot_list *)malloc(sizeof(Allot_list));
           alnode->size=allot;
           alnode->start=retptr;
           alnode->next=alptr;//Dumping into allot list
           alptr=alnode;
           
        }
        else
        {
            fib_pos=get_index_of_fib(ptr->size)-1;
            //Need to divide the list,iteratively
            Divide(&flptr,&ptr,&prev,fib_pos);
            retptr=Allot(size,H,&flptr,&alptr);
        }
    }
    *pflptr=flptr;
    *palptr=alptr;
    return retptr;
}

void print_list_status(Free_list *flptr,Allot_list *alptr)
{
    Heap_Node *ptr;
    //Allot_list *al;
    printf("\n------------Status of free list(s)-----------------");
    if(flptr==NULL)
    {
        printf("\nEMPTY");
    }
    while(flptr!=NULL)
    {
        //printf("\n");
        printf("\nSize:%d",flptr->size);
        ptr=flptr->next;
        while(ptr!=NULL)
        {
            printf("  B");//just to check no.of blocks
            //printf("-->StartAddress:%d",ptr->start);
            ptr=ptr->next;
        }
        flptr=flptr->down;
    }
    printf("\n------------Status of Allot List-------------------\n");
    if(alptr==NULL)
    {
        printf("EMPTY!!");
    }
    while(alptr!=NULL)
    {

        printf("-->Alloted Size:%d",alptr->size);
        alptr=alptr->next;
    }
    printf("\n");

}


void main()
{
    Heap H;
    Free_list *flptr=NULL;
    Allot_list *alptr=NULL;
    
    Init(H,&flptr,&alptr);
    Generate_Fibo_Arr();
    print_list_status(flptr,alptr);
    Allot(3,H,&flptr,&alptr);
    Allot(3,H,&flptr,&alptr);
    Allot(3,H,&flptr,&alptr);
    
    print_list_status(flptr,alptr);
}
