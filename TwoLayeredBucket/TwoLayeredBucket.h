#ifndef TwoLayeredBucket_h
#define TwoLayeredBucket_h

#include <stdbool.h>
#include "DualyLinkedList.h"

//////////////////////////////////////////////////
typedef enum TLBOption {
    TLB_OPTION_NONE,
    TLB_OPTION_WITH_ELEMENT
} TBL_OPTION_e;

typedef struct BucketElement {
    int value;
    void *element;
} TLBE_t;

typedef struct BucketLayer {
    int layerNo;                // Top <- 0,1,2 ...,n-1 -> Bottom
    int beginValue;
    int range;                  // end value - begin value + 1
    
    int minPos;                 // index of cell including min. value.
    int beginOfminPos;          // begin value of cell including min. value.
    int num;                    // number of elements including lower layer.
    
    int lengthOfArray;
    int rangeOfCell;
    DLL_t **array;              // array of buckets.
    
    struct BucketLayer *next;
} TLB_t;

//////////////////////////////////////////////////
TLB_t *createTLBucket(int beginValue, int endValue);
TLB_t *createLayer(int layerNo, int beginValue, int endValue, int rangeOfCell);
bool destroyTLBucket(TLB_t *B, TBL_OPTION_e option);
bool insertIntoTLBucket(TLB_t *B, int value, void *element);
bool insertBucketElementIntoTLBucket(TLB_t *B, TLBE_t *element);
void *pullMinOnTLBucket(TLB_t *B);
int findNextOnTheLayer(TLB_t *B);
void changeBottomLayer(TLB_t *B, int index);
void convertBottomLayerToList(TLB_t *B, DLL_t *list);
void convertListToBottomLayer(TLB_t *B, int index);
bool isEmptyTLBucket(TLB_t *B);
TLBE_t *createBucketElement(int value, void *elemet);
bool destroyBucketElement(TLBE_t *element);

#endif
