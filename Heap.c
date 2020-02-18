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
char * Allot(int size,Heap H,Free_list **pflptr,Allot_list **palptr)//To add malloc fail conditions(fragmentation and oversize block)
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
    if(size>1)
    {
        while(Fib[i]<size&&i<MAX_FIB_POS)
        {
            i++;
        }
        if(i<MAX_FIB_POS)
        {
            allot=Fib[i];//checks required for i>MAX_FIB_POS and when fragmentation makes call fail...
            buddy=Fib[i-1];
        }
        else
        {
            allot=Fib[MAX_FIB_POS-1]+1;//Will never be available
        }
        ptr=flptr;
        prev=NULL;
        //Search for an existing free list of required size or a list with just greater size
        while(ptr->size<allot&&ptr->down!=NULL)
        {
            prev=ptr;
            ptr=ptr->down;
        }
        if(ptr->down==NULL&&ptr->size<allot)
        {
            //Error,cant give memory,contiguous memory not available
            retptr=NULL;
            printf("ERROR....Insufficient Storage");
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
    }
    else
    {
        printf("Requested memory is too less to keep track of!!(Not Alloted)");
        //if still want to give ,call retptr=Allot(Fib[get_index_of_fib(size)+1],H,pflptr,palptr);
        retptr=NULL;
    }
    *pflptr=flptr;
    *palptr=alptr;
    return retptr;
}

void print_list_status(Free_list *flptr,Allot_list *alptr)
{
    Heap_Node *ptr;
    //Allot_list *al;
    printf("\n------------Status of Free list(s)-----------------");
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

void Merge(Free_list **pflptr,Heap_Node **ph,int fib_pos,Allot_list *alptr)//returns the address for the new merged node
{
    Heap_Node *h,*theap,*theap_prev=NULL,*new;
    Free_list *flptr,*tptr,*prev=NULL,*tmp;
    int done=0;
    flptr=*pflptr;
    h=*ph;
    h->next=NULL;
    tptr=flptr;
    if(flptr==NULL)
    {
        tmp=(Free_list *)malloc(sizeof(Free_list));
        tmp->down=NULL;
        tmp->size=Fib[fib_pos];
        tmp->next=*ph;
        flptr=tmp;
        *pflptr=flptr;
    }
    else
    {
        if(fib_pos<MAX_FIB_POS-1&&tptr!=NULL)
        {
            prev=NULL;
            while(tptr->down!=NULL&&tptr->size<Fib[fib_pos+1])
            {
                prev=tptr;
                tptr=tptr->down;
            }
            if(tptr->size==Fib[fib_pos+1])
            {
                //merging maybe possible if contiguous blocks are available
                theap=tptr->next;
                theap_prev=NULL;
                while(theap!=NULL&&done==0)
                {
                    if(theap->start==h->start+Fib[fib_pos]*sizeof(char))//Merge possible
                    {
                        done=1;//rather found
                        new=(Heap_Node *)malloc(sizeof(Heap_Node));
                        new->start=h->start;
                        new->next=NULL;
                        //delete theap
                        if(theap_prev!=NULL)
                        {
                            theap_prev->next=theap->next;
                            free(theap);
                        }
                        else
                        {
                            if(theap->next==NULL)//Delete tptr as well
                            {
                                if(prev!=NULL)
                                {
                                    prev->down=tptr->down;
                                    free(theap);
                                    free(tptr);
                                }
                                else
                                {
                                    flptr=tptr->down;
                                    free(tptr);
                                    free(theap);
                                }
                            }   
                            else
                            {
                                tptr->next=theap->next;
                                free(theap);
                            }
                        }
                        //insert into the freelist
                    }
                    else if(theap->start+Fib[fib_pos]*sizeof(char)==h->start)//Merge possible
                    {
                        done=1;//rather found
                        new=(Heap_Node *)malloc(sizeof(Heap_Node));
                        new->start=theap->start;
                        new->next=NULL;
                        //delete theap
                        if(theap_prev!=NULL)
                        {
                            theap_prev->next=theap->next;
                            free(theap);
                        }
                        else
                        {
                            if(theap->next==NULL)//Delete tptr as well
                            {
                                if(prev!=NULL)
                                {
                                    prev->down=tptr->down;
                                    free(theap);
                                    free(tptr);
                                }
                                else
                                {
                                    flptr=tptr->down;
                                    free(tptr);
                                    free(theap);
                                }
                            }   
                            else
                            {
                                tptr->next=theap->next;
                                free(theap);
                            }
                        }
                        //insert into the freelist
                    }
                    if(done==0)
                    {
                        theap_prev=theap;
                        theap=theap->next;
                    }
                }
                if(done==1)
                {
                    *pflptr=flptr;
                    free(*ph);
                    *ph=new;
                    Merge(pflptr,ph,fib_pos+2,alptr);
                }
            }   
            if(done==0)//for smaller buddy
            {
                tptr=flptr;
                prev=NULL;
                while(tptr->down!=NULL&&tptr->size<Fib[fib_pos-1])
                {
                    prev=tptr;
                    tptr=tptr->down;
                }
                if(tptr->size==Fib[fib_pos-1])
                {
                    //merging maybe possible if contiguous blocks are available
                    theap=tptr->next;
                    theap_prev=NULL;
                    while(theap!=NULL&&done==0)
                    {
                        if(theap->start==h->start+Fib[fib_pos]*sizeof(char))//Merge possible
                        {
                            done=1;//rather found
                            new=(Heap_Node *)malloc(sizeof(Heap_Node));
                            new->start=h->start;
                            new->next=NULL;
                            //delete theap
                            if(theap_prev!=NULL)
                            {
                                theap_prev->next=theap->next;
                                free(theap);
                            }
                            else    
                            {
                                if(theap->next==NULL)//Delete tptr as well
                                {
                                    if(prev!=NULL)
                                    {
                                        prev->down=tptr->down;
                                        free(theap);
                                        free(tptr);
                                    }
                                    else
                                    {
                                        flptr=tptr->down;
                                        free(tptr);
                                        free(theap);
                                    }
                                }
                                else
                                {
                                    tptr->next=theap->next;
                                    free(theap);
                                }
                            }
                            //insert into the freelist
                        }
                        else if(theap->start+Fib[fib_pos-1]*sizeof(char)==h->start)//Merge possible
                        {
                            done=1;//rather found
                            new=(Heap_Node *)malloc(sizeof(Heap_Node));
                            new->start=theap->start;
                            new->next=NULL;
                            //delete theap
                            if(theap_prev!=NULL)
                            {
                                theap_prev->next=theap->next;
                                free(theap);
                            }
                            else
                            {
                                if(theap->next==NULL)//Delete tptr as well
                                {
                                    if(prev!=NULL)
                                    {
                                        prev->down=tptr->down;
                                        free(theap);
                                        free(tptr);
                                    }
                                    else
                                    {
                                        flptr=tptr->down;
                                        free(tptr);
                                        free(theap);
                                    }
                                }
                                else
                                {
                                    tptr->next=theap->next;
                                    free(theap);
                                }
                            }
                            //insert into the freelist
                        }
                        if(done==0)
                        {
                            theap_prev=theap;
                            theap=theap->next;
                        }
                    }
                    if(done==1)
                    {
                        *pflptr=flptr;
                        free(*ph);
                        *ph=new;
                        //print_list_status(flptr,alptr);
                        Merge(pflptr,ph,fib_pos+1,alptr);
                    }
                }
            }   
            if(done==0)//no further merge possible
            {
                tptr=flptr;
                prev=NULL;
                while(tptr->down!=NULL&&tptr->size<Fib[fib_pos])
                {
                    prev=tptr;
                    tptr=tptr->down;
                }
                if(tptr->size==Fib[fib_pos])
                {
                    (*ph)->next=tptr->next;
                    tptr->next=*ph;
                }
                else 
                {
                    tmp=(Free_list *)malloc(sizeof(Free_list));
                    tmp->size=Fib[fib_pos];
                    tmp->next=*ph;
                    tmp->down=NULL;
                    if(tmp->size<tptr->size)
                    {
                        if(prev!=NULL)
                        {
                            tmp->down=tptr;
                            prev->down=tmp;
                        }
                        else
                        {
                            tmp->down=tptr;
                            flptr=tmp;
                            *pflptr=flptr;
                        }
                    }
                    else
                    {
                        tptr->down=tmp;//tmp can be previous to tptr
                        if(prev!=NULL)
                        {
                            tmp->down=prev->down;
                            prev->down=tmp;
                        }
                    }
                }
            }
        }
        if(fib_pos==MAX_FIB_POS-1)//if flptr==NULL?
        {
            tptr=flptr;
            prev=NULL;
           /*if(tptr!=NULL)
            {
                while(tptr->down!=NULL&&tptr->size<Fib[fib_pos])
                {
                    prev=tptr;
                    tptr=tptr->down;
                }
                if(tptr->size==Fib[fib_pos])
                {
                    (*ph)->next=tptr->next;
                    tptr->next=*ph;
                }
                else
                {
                    tmp=(Free_list *)malloc(sizeof(Free_list));
                    tmp->size=Fib[fib_pos];
                    tmp->next=*ph;
                    //tptr->down=tmp;
                    if(prev!=NULL)
                    {
                        tmp->down=prev->down;
                        prev->down=tmp;
                    }
                }
            }*/
            //else
            //{
            tmp=(Free_list *)malloc(sizeof(Free_list));
            tmp->size=Fib[fib_pos];
            tmp->next=*ph;
            flptr=tmp;
            *pflptr=flptr;
            //}
        }
    }
}

void FreeUp(Free_list **pflptr,Allot_list **palptr,char *ptr)
{
    //search the element in alloc list
    Free_list *flptr,*tptr;
    Allot_list *alptr,*alnode,*aprev=NULL,*tmp;
    Heap_Node *h;
    h=(Heap_Node *)malloc(sizeof(Heap_Node));
    int size,fib_pos;
    alptr=*palptr;
    flptr=*pflptr;
    alnode=alptr;
    while(alnode!=NULL&&alnode->start!=ptr)
    {
        aprev=alnode;
        alnode=alnode->next;
    }
    tmp=alnode;
    size=tmp->size;
    if(aprev!=NULL)
    {
        aprev->next=tmp->next;
    }
    else
    {
        alptr=tmp->next;
    }
    free(tmp);
    h->start=ptr;
    h->next=NULL;
    fib_pos=get_index_of_fib(size);
    Merge(pflptr,&h,fib_pos,alptr);
    *palptr=alptr;
}

void main()
{
    Heap H;
    Free_list *flptr=NULL;
    Allot_list *alptr=NULL;
    char *s,*a,*b,*c,*d;
    Init(H,&flptr,&alptr);
    Generate_Fibo_Arr();
    print_list_status(flptr,alptr);
    
    /*a=Allot(3,H,&flptr,&alptr);
    //c=Allot(2,H,&flptr,&alptr);
    s=Allot(3,H,&flptr,&alptr);
    print_list_status(flptr,alptr);
    //To check and write correctly
    FreeUp(&flptr,&alptr,c);
    print_list_status(flptr,alptr);
    */
    /*s=Allot(3,H,&flptr,&alptr);
    a=Allot(4,H,&flptr,&alptr);
    d=Allot(3,H,&flptr,&alptr);
    print_list_status(flptr,alptr);
    c=Allot(3,H,&flptr,&alptr);
    print_list_status(flptr,alptr);
    FreeUp(&flptr,&alptr,s);
    print_list_status(flptr,alptr);
    FreeUp(&flptr,&alptr,d);
    print_list_status(flptr,alptr);
    FreeUp(&flptr,&alptr,a);
    print_list_status(flptr,alptr);
    */
    c=Allot(7,H,&flptr,&alptr);
    d=Allot(3,H,&flptr,&alptr);
    print_list_status(flptr,alptr);
    FreeUp(&flptr,&alptr,d);
    print_list_status(flptr,alptr);
    FreeUp(&flptr,&alptr,c);
    print_list_status(flptr,alptr);
    c=Allot(1,H,&flptr,&alptr);
    //FreeUp(&flptr,&alptr,c);
    print_list_status(flptr,alptr);
    //s=Allot(3,H,&flptr,&alptr);
    //a=Allot(3,H,&flptr,&alptr);
    //b=Allot(3,H,&flptr,&alptr);
    //c=Allot(2,H,&flptr,&alptr);
    //d=Allot(2,H,&flptr,&alptr);
    //print_list_status(flptr,alptr);
    //FreeUp(&flptr,&alptr,c);

    //print_list_status(flptr,alptr);
}
