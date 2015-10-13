/*	$Id: btree.h,v 1.2 2013/02/28 21:25:00 lombard Exp $	*/

/*
 * btree.h--
 *    structure Btree represents a binary tree. It is used in
 *    various places to speed up searching.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved. 
 */
#ifndef BTREE_H
#define BTREE_H

typedef struct bt_node_ {
    int numVal;
    int maxVal;
    void **val;
    struct bt_node_ *left, *right;
} BtreeNode;

typedef struct bt_root_ {
    int numElt;
    BtreeNode *root;
    int (*keycmp)();
} Btree;


#endif BTREE_H
