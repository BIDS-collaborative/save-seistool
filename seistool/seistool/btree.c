#ifndef lint
static char id[] = "$Id: btree.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * btree.c--
 *    implementation of the abstract data type Btree
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include "proto.h"

static BtreeNode *makebtreenode(void *val);
static void btree_printNode(BtreeNode *n, void (*prn_func)());
static void btree_linNode(BtreeNode *n, BtreeNode **nodes, int *iptr);
static void btree_destroyNode(BtreeNode *n);


Btree *empty_btree(int(*cmp_func)())
{
    Btree *bt = (Btree *)Malloc(sizeof(Btree));
    bt->numElt= 0;
    bt->keycmp= cmp_func;
    bt->root= NULL;
    return bt;
}

static BtreeNode *makebtreenode(void *val)
{
    BtreeNode *node= (BtreeNode *)Malloc(sizeof(BtreeNode));
    node->numVal= 1;
    node->maxVal= 8;
    node->val= (void **)Malloc(sizeof(void *)*8);
    node->val[0]= val;
    node->left= node->right= NULL;
    return node;
}

void btree_insert(Btree *bt, void *val)
{
    BtreeNode *node, *y= NULL;
    BtreeNode *rt= bt->root;
    while(rt) {
	int cmp= bt->keycmp(rt->val[0], val);
	y= rt;
	if(cmp==0) {
	    /* key already in the tree */
	    rt->val[rt->numVal++]= val;
	    if(rt->numVal==rt->maxVal) {
		rt->maxVal *= 2;
		rt->val= (void **)Realloc(rt->val, sizeof(void *) *
					  rt->maxVal);
	    }
	    return;
	}else if (cmp>0) {
	    rt= rt->left;
	}else {
	    rt= rt->right;
	}
    }
    node= makebtreenode(val);
    if(y==NULL) {
	bt->root= node;
    }else {
	int cmp2;
	if( (cmp2 = bt->keycmp(y->val[0], val)) > 0) {
	    y->left= node;
	}else {
	    y->right= node;
	}
    }
    bt->numElt++;
}

static void btree_printNode(BtreeNode *n, void (*prn_func)())
{
    if(!n) return;
    btree_printNode(n->left, prn_func);
    prn_func(n->numVal, n->val);
    btree_printNode(n->right, prn_func);
}

void btree_print(Btree *bt, void (*prn_func)())
{
    btree_printNode(bt->root, prn_func);
}

static void btree_linNode(BtreeNode *n, BtreeNode **nodes, int *iptr)
{
    if(!n) return;
    btree_linNode(n->left, nodes, iptr);
    nodes[(*iptr)++]= n;
    btree_linNode(n->right, nodes, iptr);
}

BtreeNode **btree_linearize(Btree *bt)
{
    int index=0;
    BtreeNode **nodes=(BtreeNode **)Malloc(sizeof(BtreeNode*)*bt->numElt);
    btree_linNode(bt->root, nodes, &index);
    return nodes;
}

static void btree_destroyNode(BtreeNode *n)
{
    if(!n)return;
    btree_destroyNode(n->left);
    free(n->val);
    btree_destroyNode(n->right);
    free(n);
}

void btree_destroy(Btree *bt)
{
    btree_destroyNode(bt->root);
    free(bt);
}
