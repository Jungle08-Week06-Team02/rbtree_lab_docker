#include "rbtree.h"
#include <stdlib.h>

rbtree *new_rbtree(void) {

  // TODO: initialize struct if needed
  // RB tree 구조체 생성
  // 여러 개의 tree를 생성할 수 있어야 하며 각각 다른 내용들을 저장할 수 있어야 합니다.

  // 1) Sentinel(NIL) 노드 만들기
  // 2) 트리 구조체 초기화(모든 노드의 자식이 t->nil을 가리키도록)

  /* t를 포인터로 만드는 이유
  포인터(rbtree *)로 만들어야만 동작이 확장 가능하고, 다수 트리를 동시에 안전하게 다룰 수 있음
  값(rbtree t)으로 만들면 구조가 제한되고, 메모리 해제나 동적 처리도 어려움
  1) 동적으로 크기를 관리하고, 여러 트리를 만들 수 있어야 하기 때문
  2) 함수에서 구조체 전체를 반환하면 복사본이 생김 (값 반환 vs 포인터 반환)
    포인터를 써야 진짜 트리 안의 노드 조작 가능, 값 복사한 거면 트리 내부 상태를 바꿔도 복사본만 수정되고 진짜 트리 그대로
  3) free()도 오직 힙 메모리에만 가능 */

  // sentinel 노드의 key를 초기화하진 않는 이유? : nil인지 아닌지만 검사하기 때문에 key로 접근할 일 없음
  // root 노드의 parent는 nil

  rbtree *t = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *nil = (node_t *)calloc(1, sizeof(node_t));
  nil->color = RBTREE_BLACK;
  nil->left = nil->right = nil->parent = NULL;
  
  t->nil = nil;
  t->root = t->nil;

  return t;

}


// 전체 트리 삭제시에만 호출하는 내부 함수 구현
static void _delete_node(rbtree *t, node_t *node) {

  // 전위, 중위, 후위 순회 시, 좌 → 우로 부르는 재귀구조는 동일
  // 프린트 or free가 호출되는 시점이 순회를 결정하는데, 작업이 실행되는 시점의 노드 순서가 핵심임
  // 이 함수의 경우 자녀들보다 부모가 먼저 free되면 자녀들에게 접근할 방법이 없어지므로, 무조건 후위 순회를 해야 함

  // static : 내부 전용, 파일 내부 전용 헬퍼 함수를 만들 때 해당 .c 파일 안에서만 사용 가능하게 함
  // extern : 외부 함수, 다른 .c 파일에서 정의된 함수임을 선언만 함 (보통 .h에 씀)
  // inline : 성능 최적화 힌트, 짧은 함수라면 함수 호출 없이 코드 복사하라는 힌트 (컴파일러가 무시할 수도 있음)

  if (node == t->nil) return; // sentinel은 나중에 삭제
  _delete_node(t, node->left); 
  _delete_node(t, node->right);
  free(node);
}


void delete_rbtree(rbtree *t) {

  // TODO: reclaim the tree nodes's memory
  // RB tree 구조체가 차지했던 메모리 반환
  // 해당 tree가 사용했던 메모리를 전부 반환해야 합니다. (valgrind로 나타나지 않아야 함)

  // 1) 트리에 있는 모든 노드를 후위 순회로 free() -> 자식을 먼저 지우고 부모 지움
  // 2) 마지막에 t->nil도 free()
  // 3) 트리 자체도 free()
  
  // C언어에서는 중첩함수 불가하므로(함수는 무조건 최상위 스코프(top-level)에서만 정의 가능), _delete_node 따로 만들어서 부르기

  _delete_node(t, t->root);
  free(t->nil);
  free(t);

}


// left 회전 내부 함수 구현
static void _left_rotate(rbtree *t, node_t *x) {

  node_t *y = x->right;
  x->right = y->left;
  
  if(y->left != t->nil) y->left->parent = x;

  y->parent = x->parent;

  if(x->parent == t->nil) t->root = y;
  else if(x == x->parent->left) x->parent->left = y;
  else x->parent->right = y;

  y->left = x;
  x->parent = y;
}


// right 회전 내부 함수 구현 
static void _right_rotate(rbtree *t , node_t *x) {

  node_t *y= x->left;
  x->left = y->right;
  
  if(y->right != t->nil) y->right->parent = x;

  y->parent = x->parent;
  
  if(x->parent == t->nil) t->root = y;
  else if(x == x->parent->right) x->parent->right = y;
  else x->parent->left = y;

  y->right = x;
  x->parent = y;
}

// 삽입으로 조건 위배 시 고치는 내부 함수 구현 
static void _rb_insert_fixup(rbtree *t, node_t *z) {
  
  node_t *y = NULL;

  while (z->parent->color == RBTREE_RED) // B(G)-R(P)-R(N) 상황
  {
    if (z->parent == z->parent->parent->left) // 아부지가 할부지 왼팔이면
    {
      // 삼촌은 할부지 오른팔
      y = z->parent->parent->right;

      // 삼촌도 R이면 -> B(G), R(P, U, N)인 상태이므로 case 1
      if (y->color == RBTREE_RED)
      { 
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent; // 할부지부터 다시 검사 -> while 걸릴 수도, 안 걸릴 수도  
      }

      // 삼촌이 B이면, 허리 핀 경우와 허리 굽은 경우 판단
      else
      {
        if (z == z->parent->right) // 허리 굽은 경우(<, LR) case 2
        {
          z = z->parent;
          _left_rotate(t, z);
        }

        // 허리 핀 경우(/, LL) case 1 (case 2를 해결하고 온 후에도 걸림)
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        _right_rotate(t, z->parent->parent);
      }
    } 
    
    else // 아부지가 할부지 오른팔이면
    {
      // 삼촌은 왼팔
      y = z->parent->parent->left;

      // 삼촌도 R이면 -> B(G), R(P, U, N)인 상태이므로 case 1
      if (y->color == RBTREE_RED)
      { 
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent; // 할부지부터 다시 검사 -> while 걸릴 수도, 안 걸릴 수도  
      }

      // 삼촌이 B이면, 허리 핀 경우와 허리 굽은 경우 판단
      else
      {
        if (z == z->parent->left) // 허리 굽은 경우(>, RL) case 2
        {
          z = z->parent;
          _right_rotate(t, z);
        }

        // 허리 핀 경우(\, RR) case 1 (case 2를 해결하고 온 후에도 걸림)
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        _left_rotate(t, z->parent->parent);
      }
    }
  }  
  // 2번 조건(루트는 블랙) 위배 시 그냥 블랙으로 바꿔줘서 처리하면 끝  
  t->root->color = RBTREE_BLACK; 
}


node_t *rbtree_insert(rbtree *t, const key_t key) {  
  
  // TODO: implement insert
  // key 추가
  // 구현하는 ADT가 multiset이므로 이미 같은 key의 값이 존재해도 하나 더 추가합니다.

  // malloc은 void* 반환 → 포인터 변수로 받아야 하며, 자동 형변환됨
  // C에서는 void*가 자동 형변환되기 때문에 명시적 형변환은 생략 가능하지만, C++에서는 명시적 형변환 필요
 
  node_t *z = (node_t *)calloc(1, sizeof(node_t));

  z->key = key;
  z->color = RBTREE_RED;
  node_t *x = t->root;
  node_t *y = t->nil;

  while (x != t->nil) 
  {
    y = x;
    if (z->key < x->key) x = x->left;
    else x = x->right;
  }

  z->parent = y;
  
  if (y == t->nil) t->root = z;
  else if (z->key < y->key) y->left = z;
  else y->right = z;

  z->left = t->nil;
  z->right = t->nil;
  _rb_insert_fixup(t, z);
  return z;
} 


node_t *rbtree_find(const rbtree *t, const key_t key) {

  // TODO: implement find
  // RB tree내에 해당 key가 있는지 탐색하여 있으면 해당 node pointer 반환
  // 해당하는 node가 없으면 NULL 반환
 
  node_t *x = t->root;

  while (x != t->nil) {
    if (key == x->key) return x;
    if (key < x->key) x = x->left;
    else x = x->right;
  }

  return NULL;

}


node_t *rbtree_min(const rbtree *t) {

  // TODO: implement find
  // RB tree 중 최소 값을 가진 node pointer 반환

  // y 안 쓴 ver.
  // if (t->root == t->nil) return NULL;
  // node_t *x = t->root;
  // while (x->left != t->nil)
  //   x = x->left;
  // return x;

  node_t *x = t->root;
  node_t *y = t->nil;
  
  while (x != t->nil) {
    y = x;
    x = x->left;
  }

  return y == t->nil ? NULL : y;

}


node_t *rbtree_max(const rbtree *t) {

  // TODO: implement find
  // RB tree 중 최대 값을 가진 node pointer 반환

  // y 안 쓴 ver.
  // if (t->root == t->nil) return NULL;
  // node_t *x = t->root;
  // while (x->right != t->nil)
  //   x = x->right;
  // return x;

  node_t *x = t->root;
  node_t *y = t->nil;
  
  while (x != t->nil) {
    y = x;
    x = x->right;
  }

  return y == t->nil ? NULL : y;

}


// 삭제를 위해 successor 같은 거 찾았을 때 자리 교환하는 내부 함수 구현
// u : 삭제할 노드
// v : u를 대체할 노드. 리프일 수도 있고, T.nil일 수도 있음
// 부모 포인터(v->parent)는 갱신하지만, v의 자식 포인터(v->left, v->right)는 갱신 안함
static void _rb_transplant(rbtree *t, node_t *u, node_t *v) {
  if (u->parent == t->nil) t->root = v;
  else if (u == u->parent->left) u->parent->left = v;
  else u->parent->right = v;
  v->parent = u->parent;
}


// 지우려고 하는 노드의 successor를 찾는 내부 함수 구현
static node_t *_subtree_min(const rbtree *t, node_t *node) {
  while (node->left != t->nil)
    node = node->left;
  return node;
}


// 삭제 시 조건 위반 생길 경우 수정하는 내부 함수 구현
static void _rb_delete_fixup(rbtree *t, node_t *x) {
  
  node_t *w = t->nil;
  
  while ((x != t->root) && (x->color == RBTREE_BLACK)) 
  {
    if (x == x->parent->left) // 나(DB) 왼쪽, 형제는 오른쪽
    {
      w = x->parent->right;
      
      if (w->color == RBTREE_RED) // case1 (형제는 red)
        {
          w->color = RBTREE_BLACK;
          x->parent->color = RBTREE_RED;
          _left_rotate(t, x->parent);
          w = x->parent->right; // rotate 후, x의 형제가 바뀌었으니 w로 가리키는 곳 업데이트
        }
      
      if ((w->left->color == RBTREE_BLACK)&&(w->right->color == RBTREE_BLACK)) // case2 (형제, 형제 자식 블랙)
        {  
        w->color = RBTREE_RED;
        x = x->parent;
        }
      
      else // 형제는 black, 형제의 자식 중 red가 있을 경우
      {
      
        if (w->right->color == RBTREE_BLACK) // case3 (형제의 왼쪽 자식이 red, 나는 현재 왼쪽 자식이므로 더 가까운 조카가 빨강)
        {
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          _right_rotate(t, w);
          w = x->parent->right;
        }
      
        // case4 (형제의 오른쪽 자식이 red, 나와 먼 조카)
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        _left_rotate(t, x->parent);
        x = t->root; // case4의 해결로 모든 조건이 충족됐으므로 while 문 탈출을 위한 조건을 걸어줌
      }
    }

    else // 나(DB) 오른쪽, 형제는 왼쪽
    {
      w = x->parent->left;
      
      if (w->color == RBTREE_RED) // case1 (형제는 red)
        {
          w->color = RBTREE_BLACK;
          x->parent->color = RBTREE_RED;
          _right_rotate(t, x->parent);
          w = x->parent->left; // rotate 후, x의 형제가 바뀌었으니 w로 가리키는 곳 업데이트
        }
      
      if ((w->right->color == RBTREE_BLACK)&&(w->left->color == RBTREE_BLACK)) // case2 (형제, 형제 자식 블랙)
        {  
        w->color = RBTREE_RED;
        x = x->parent;
        }
      
      else // 형제는 black, 형제의 자식 중 red가 있을 경우
      {
      
        if (w->left->color == RBTREE_BLACK) // case3 (형제의 오른쪽 자식이 red, 나는 현재 오른쪽 자식이므로 더 가까운 조카가 빨강)
        {
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          _left_rotate(t, w);
          w = x->parent->left;
        }
      
        // case4 (형제의 왼쪽 자식이 red, 나와 먼 조카)
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        _right_rotate(t, x->parent);
        x = t->root; // case4의 해결로 모든 조건이 충족됐으므로 while 문 탈출을 위한 조건을 걸어줌
      }
    }

  }
  x->color = RBTREE_BLACK; // 계속 위로 올라가서 x가 루트가 됐을 경우, 루트 블랙 조건 보장
}


int rbtree_erase(rbtree *t, node_t *p) {
  
  // TODO: implement erase
  // RB tree 내부의 ptr로 지정된 node를 삭제하고 메모리 반환
  // test 조건엔 없지만 삭제하는 노드 key 반환하도록 해뒀음 
  
  node_t *x = t->nil; // y 자리를 대체할 노드
  node_t *y = p; // 실제 삭제되는 노드 (처음엔 p 자신, 나중엔 successor가 될 수도 있음)
  color_t y_original_color = y->color;

  // case 1: 왼쪽 자식이 없음 → 오른쪽 자식으로 대체 (혹은 nil)
  if (p->left == t->nil) {
    x = p->right;
    _rb_transplant(t, p, p->right);
  }

  // case 2: 오른쪽 자식이 없음 → 왼쪽 자식으로 대체
  else if (p->right == t->nil) {
    x = p->left;
    _rb_transplant(t, p, p->left);
  }

  // case 3: 양쪽 자식 모두 있음
  else {
    // 후계자 y = 오른쪽 서브트리의 최소 노드
    y = _subtree_min(t, p->right);
    y_original_color = y->color;
    x = y->right;

    // case 3-1: y가 p의 직속 오른쪽 자식이 아님
    if (y != p->right) {
      // y 자리를 y의 오른쪽 자식으로 대체
      _rb_transplant(t, y, y->right);

      // y에 p의 오른쪽 자식 연결
      y->right = p->right;
      y->right->parent = y;
    } else {
      // case 3-2: y가 p의 직속 오른쪽 자식 → transplant 생략했으므로 직접 부모 연결
      x->parent = y;
    }

    // p 자리를 y로 대체 (y는 이제 p의 위치로 올라옴)
    _rb_transplant(t, p, y);

    // y에 p의 왼쪽 자식 연결
    y->left = p->left;
    y->left->parent = y;

    // 삭제 전 p의 색을 y에게 전달 (트리 속성 유지용)
    y->color = p->color;
  }

  // 🔧 삭제된 노드가 BLACK일 경우 → 속성 위배 가능성 → fixup 필요
  if (y_original_color == RBTREE_BLACK) {
    _rb_delete_fixup(t, x);
  }

  // 삭제된 노드 메모리 해제
  free(p);
  p = NULL;
  return 0;
}


// 중위순회(n만큼) 내부 함수 구현
static void _inorder_tree(const rbtree *t, node_t *node, key_t *arr, size_t n, int *idx) {
  
  if (node == t->nil || *idx >= n) return;

  _inorder_tree(t, node->left, arr, n, idx); 
  if (*idx < n) arr[(*idx)++] = node->key; 
  _inorder_tree(t, node->right, arr, n, idx);

}


int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  
  // TODO: implement to_array
  // RB tree의 내용을 *key 순서대로* 주어진 array로 변환
  // array의 크기는 n으로 주어지며 tree의 크기가 n 보다 큰 경우에는 순서대로 n개 까지만 변환
  // array의 메모리 공간은 이 함수를 부르는 쪽에서 준비하고 그 크기를 n으로 알려줍니다.
  
  int idx = 0;
  _inorder_tree(t, t->root, arr, n, &idx);

  return idx;

}
