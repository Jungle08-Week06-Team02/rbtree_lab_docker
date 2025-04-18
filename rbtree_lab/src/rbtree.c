#include "rbtree.h"
#include <stdlib.h>

void delete_node(rbtree *t, node_t *n_t);
void left_rotate(rbtree *t, node_t *x);
void right_rotate(rbtree *t , node_t *x);


rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  // rbtree 구조체를 가리키는 포인터를 rbtree의 사이즈 * 1만큼 동적 활당
  node_t *nil_node = (node_t*)calloc(1, sizeof(node_t)); 
  // node_t 구조체를 가리키는 포인터 node_t의 사이즈 * 1만큼 동적 할당
  nil_node->color = RBTREE_BLACK;// nil노드의 색을 BLACK으로 설정
  //nil노드의 구조체 초기화
  nil_node->left = NULL;
  nil_node->right = NULL;
  nil_node->parent = NULL;

  p->nil  = nil_node;
  p->root = nil_node;

  return p;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  // 트리를 전부 순회하면서 각 노드를의 메모리 할당을 지워야 하는건가?
  // rbtree_erase를 사용해서 해당 트리의 root가 nil_node가 될때까지 root노드를 삭제한다.
  // 위의 생각은 트리를 재조정이 들어가서 비효율적, 트리를 순회하면서 노드의 메모리 할당을 해제한다.
  // 루트 노드를 마지막에 할당해제 하고 nil노드를 할당해제한다.
  delete_node(t, t->root);
  free(t->nil);
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // TODO: implement erase
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}


//////////////////////////////////////////////////////

static void delete_node(rbtree *t, node_t *n_t)
{ //트리를 후위 순회하면서 노드의 메모리 해제
  if(n_t == t->nil) //현재 노드가 nil노드라면 
    return;

  delete_node(t, n_t->left);
  delete_node(t, n_t->right);
  free(n_t);
}

void left_rotate(rbtree *t, node_t *x)
{
  node_t *y = x->right;
  x->right = y->left;
  
  if(y->left != t->nil)
    y->left->parent = x;

  y->parent = x->parent;

  if(x->parent == t->nil)
    t->root = y;
  else if(x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;

  y->left = x;
  x->parent = y;
}

void right_rotate(rbtree *t , node_t *x)
{
  node_t *y= x->left;
  x->left = y->right;
  
  if(y->right != t->nil)
    y->right->parent = x;

  y->parent = x->parent;
  
  if(x->parent == t->nil)
    t->root = y;
  else if(x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;

  y->right = x;
  x->parent = y;
}