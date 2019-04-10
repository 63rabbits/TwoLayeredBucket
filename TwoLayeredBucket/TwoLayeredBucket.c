#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "TwoLayeredBucket.h"

//////////////////////////////////////////////////
TLB_t *createTLBucket(int beginValue, int endValue) {
    // create top layer.
    int C = (int)ceil(sqrt(endValue - beginValue + 1));
    TLB_t *B1 = createLayer(1, beginValue, endValue, C);
    
    // create bottom layer.
    TLB_t *B0 = createLayer(0, beginValue, beginValue + C - 1, 1);
    
    // link B1 to B0.
    B1->next = B0;
    
    return B1;
}

TLB_t *createLayer(int layerNo, int beginValue, int endValue, int rangeOfCell) {
    TLB_t *B = malloc(sizeof(TLB_t));
    if (B == NULL) return NULL;
    
    B->layerNo = layerNo;
    B->beginValue = beginValue;
    B->range = endValue - beginValue + 1;
    B->minPos = 0;
    B->beginOfminPos = B->beginValue;
    B->num = 0;
    B->lengthOfArray = (int)ceil((double)B->range / rangeOfCell);
    B->rangeOfCell = rangeOfCell;
    
    B->array = malloc(B->lengthOfArray * sizeof(void *));
    if (B->array == NULL) {
        free(B);
        return NULL;
    }
    for (int i=0; i<B->lengthOfArray; i++) {
        B->array[i] = createDLList();
    }
    
    B->next = NULL;
    
    return B;
}

bool destroyTLBucket(TLB_t *B, TBL_OPTION_e option) {
    if (B->next != NULL) {
        destroyTLBucket(B->next, option);
    }
    
    for (int i= 0; i < B->lengthOfArray; i++) {
        // * Avoid memory leaks. *
        // Since elements registered in the list point to other areas,
        // they are deleted individually.
        DLL_t *list = B->array[i];
        while (!isEmptyDLList(list)) {
            TLBE_t *bucketElement = pullDLList(list);
            if ((option == TLB_OPTION_WITH_ELEMENT) &&
                (bucketElement->element != NULL)) {
                free(bucketElement->element);
            }
            free(bucketElement);
        }
        destroyDLList(list, DLL_OPTION_NONE);
    }
    free(B->array);
    free(B);
    
    return true;
}

bool insertIntoTLBucket(TLB_t *B, int value, void *element) {
    // Block illegal parameters.
    if (B == NULL) return false;
    
    TLBE_t *bucketElement = createBucketElement(value, element);
    bool check = insertBucketElementIntoTLBucket(B, bucketElement);
    if (!check) {
        destroyBucketElement(bucketElement);
        return false;
    }
    return true;
}

bool insertBucketElementIntoTLBucket(TLB_t *B, TLBE_t *element) {
    int value = element->value;
    
    if ((value < B->beginValue) ||
        (value >= (B->beginValue + B->range))) {
        return false;
    }
    
    if (B->num <= 0) {
        changeBottomLayer(B, 0);
    }
    
    bool result = false;
    int i = (B->minPos + (int)floor((double)(value - B->beginOfminPos) / B->rangeOfCell)) % B->lengthOfArray;
    if ((i == B->minPos) && (B->next != NULL)) {    // for bottom layer.
        i = (B->next->minPos + (int)floor((double)(value - B->next->beginOfminPos) / B->next->rangeOfCell)) % B->next->lengthOfArray;
        result = insertAtTailOnDLList(B->next->array[i], element);
        if (result) {
            B->next->num++;
        }
    }
    else {  // for top layer.
        result = insertAtTailOnDLList(B->array[i], element);
    }
    if (result) {
        B->num++;
        if (value < B->beginOfminPos) {
            changeBottomLayer(B, i);
        }
    }

    return result;
}

void *pullMinOnTLBucket(TLB_t *B) {
    // Block illegal parameters.
    if (B == NULL) return NULL;
    
    if (B->next != NULL) {
        while (B->next->num > 0) {
            void *element = pullMinOnTLBucket(B->next);
            if (element != NULL) {
                B->num--;
                if (B->next->num <= 0) {
                    int next = findNextOnTheLayer(B);
                    changeBottomLayer(B, next);
                }
                return element;
            }
            int next = findNextOnTheLayer(B);
            changeBottomLayer(B, next);
        }
        return NULL;
    }
    
    // for lowest layer.
    while (B->num > 0) {
        DLL_t *list = B->array[B->minPos];
        if (!isEmptyDLList(list)) {
            TLBE_t *bucketElement = pullHeadOnDLList(list);
            B->num--;
            if (isEmptyDLList(list)) {
                B->minPos = findNextOnTheLayer(B);
                B->beginOfminPos = B->beginValue + B->minPos * B->rangeOfCell;
            }
            void *element = bucketElement->element;
            destroyBucketElement(bucketElement);
            return element;
        }
        B->minPos = findNextOnTheLayer(B);
        B->beginOfminPos = B->beginValue + B->minPos * B->rangeOfCell;
    }
    return NULL;
}

int findNextOnTheLayer(TLB_t *B) {
    for (int i=1; i<B->lengthOfArray; i++) {
        int j = (B->minPos + i) % B->lengthOfArray;
        if (!isEmptyDLList(B->array[j])) {
            return j;
        }
    }
    return B->minPos;
}

void changeBottomLayer(TLB_t *B, int index) {
    // Block illegal parameters.
    if (index == B->minPos) return;                     // already converted.
    if ((index < 0) || (index >= B->lengthOfArray)) return; // illegal index.
    
    if (B->next != NULL) {
        convertBottomLayerToList(B->next, B->array[B->minPos]);
        convertListToBottomLayer(B, index);
        return;
    }
    
    // for lowest layer.
    B->minPos = index;
    B->beginOfminPos = B->beginValue + index * B->rangeOfCell;
}

void convertBottomLayerToList(TLB_t *B, DLL_t *list) {
    // Block illegal parameters.
    if (B->num <= 0) return;
    
    // for bottom layer.
    for (int i=0; i < B->lengthOfArray; i++) {
        if (B->array[i] != NULL) {
            list = margeDLList(list, B->array[i]);
        }
    }
    B->num = 0;
    B->minPos = 0;
}

void convertListToBottomLayer(TLB_t *B, int index) {
    if (index == B->minPos) return;     // already converted.
    
    DLL_t *list = B->array[index];
    B->minPos = index;
    B->beginOfminPos = B->beginValue + index * B->rangeOfCell;
    
    // initialize bottom layere.
    B->next->beginValue = B->beginOfminPos;
    B->next->minPos = 0;
    B->next->beginOfminPos = B->next->beginValue;
    B->next->num = 0;
    
    // insert elements of list into bottom layere.
    while (!isEmptyDLList(list)) {
        TLBE_t *bucketElement = pullHeadOnDLList(list);
        insertBucketElementIntoTLBucket(B->next, bucketElement);
    }
}

bool isEmptyTLBucket(TLB_t *B) {
    if (B->num <= 0) return true;
    return false;
}

TLBE_t *createBucketElement(int value, void *elemet) {
    // Block illegal parameters.
    if (elemet == NULL) return NULL;
    
    TLBE_t *bucketElement = malloc(sizeof(TLBE_t));
    if (bucketElement == NULL) return NULL;
    bucketElement->value = value;
    bucketElement->element = elemet;
    return bucketElement;
}

bool destroyBucketElement(TLBE_t *element) {
    // Block illegal parameters.
    if (element == NULL) return false;
    free(element);
    return true;
}
