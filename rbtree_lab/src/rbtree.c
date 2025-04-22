#include "rbtree.h"
#include <stdlib.h>

static void _delete_node(rbtree *t, node_t *n_t);           //tree를 순회하면서 노드의 메모리 할당을 해제하는 함수
static void _left_rotate(rbtree *t, node_t *x);             //이진 트리의 속성을 유지하면서 왼쪽으로 회전하는 함수
static void _right_rotate(rbtree *t , node_t *x);           //이진 트리의 속성을 유지하면서 오른쪽으로 회전하는 함수
static void _rb_insert_fixup(rbtree *t, node_t *z);         //RB-Tree의 삽입 후, RED BLACK 속성 위반을 수정하는 함수
static node_t *_subtree_min(const rbtree *t, node_t *node);
static void _rb_transplant(rbtree *t, node_t *u, node_t *v);
static void _rb_delete_fixup(rbtree *t, node_t *x);
// static void _inorder_tree(const rbtree *t, node_t *node, key_t* arr, size_t n, size_t *idx);
static void _inorder_tree(const rbtree *t, node_t *node, key_t *arr, size_t n, int *idx);

rbtree *new_rbtree(void) 
{
  rbtree *tree_p = (rbtree *)calloc(1, sizeof(rbtree)); //rbtree 구조체를 가리키는 포인터를 rbtree의 사이즈 * 1만큼 동적 활당               
  node_t *nil_node = (node_t*)calloc(1, sizeof(node_t));//node_t 구조체를 가리키는 포인터 node_t의 사이즈 * 1만큼 동적 할당 
                                                      
  nil_node->color = RBTREE_BLACK;                       //nil노드의 색을 BLACK으로 초기화
  nil_node->left = NULL;
  nil_node->right = NULL;
  nil_node->parent = NULL;                                  
  
  tree_p->nil  = nil_node;                              //tree_p의 초기화
  tree_p->root = tree_p->nil;       

  return tree_p;
}

void delete_rbtree(rbtree *t) 
{
  _delete_node(t, t->root);                              //delete_node의 매개변수 t와 t->root로 delete_node 함수호출
  free(t->nil);                                         //t의 nil 노드 메모리 할당해제
  free(t);                                              //t의 메모리 할당해제
}

node_t *rbtree_insert(rbtree *t, const key_t key) 
{
// key 추가
// 구현하는 ADT가 multiset이므로 이미 같은 key의 값이 존재해도 하나 더 추가
// malloc은 void* 반환 → 포인터 변수로 받아야 하며, 자동 형변환됨
// C에서는 void*가 자동 형변환되기 때문에 명시적 형변환은 생략 가능 
  node_t *insert_node;
  node_t *compare_node = t->root;                     //비교 대상 노드
  node_t *insert_node_parent = t->nil;                //insert_node_parent는 insert_node의 부모가 될 노드

  insert_node = (node_t *)calloc(1, sizeof(node_t));  //insert_node는 key값을 가진 새로운 노드
  insert_node->key = key;                             //insert_node의 초기화
  insert_node->color = RBTREE_RED;

  while (compare_node != t->nil)                      //compare_node가 nil이 되기 전까지 반복
  {
    insert_node_parent = compare_node;                //insert_node_parent에 compare_node를 대입
    if (insert_node->key < compare_node->key)         //insert_node의 key가 compore_node의 key보다 작을 때
      compare_node = compare_node->left;              //compare_node는 left로 이동하며 더 작은 값이 된다.
    else                                              //insert_node의 key가 compore_node의 key보다 클 때
      compare_node = compare_node->right;             //compare_node는 right로 이동하며 더 큰 값이 된다.
  }                                                   //while문을 빠져나왔을 때 insert_node_parent는 nil노드를 자식으로 가지는 노드가 된다.

  insert_node->parent = insert_node_parent;           //insert_node의 부모가 insert_node_parent가 된다.
  
  if (insert_node_parent == t->nil)                   //insert_node_paret가 nil 노드라면 *처음부터 트리가 비어있는 상태
    t->root = insert_node;                            //t트리의 root를 insert_node로 한다.
  else if (insert_node->key < insert_node_parent->key)//insert_node의 key가 insert_node_parent의 키보다 작을 때
    insert_node_parent->left = insert_node;           //insert_node는 insert_node_parent의 왼쪽 자식이 된다.
  else                                                //insert_node의 key가 insert_node_parent의 키보다 작을 때
    insert_node_parent->right = insert_node;          //insert_node는 insert_node_parent의 오른쪽 자식이 된다.

  insert_node->left = t->nil;                         //insert_node의 왼쪽 자식을 nil node로 초기화            
  insert_node->right = t->nil;                        //insert_node의 오른쪽 자식을 nil node로 초기화
  _rb_insert_fixup(t, insert_node);                   //RED BLACK 특성 위반을 수정한다.

  return insert_node;                                 //insert_node의 포인터를 반환한다.
}

node_t *rbtree_find(const rbtree *t, const key_t key)  
{
  node_t *find_node = t->root;                        //find_node를 t->root를 가리키게 한다.

  while (find_node != t->nil)                         //find_node가 nil노드가 아니면 while문을 반복 
  {   
    if (key == find_node->key)                        //key의 값이 find_node의 key값이라면 
      return find_node;                               //find_node를 반환한다.
    
    if (key < find_node->key)                         //key값이 find_node의 key보다 작다면
      find_node = find_node->left;                    //find_node는 find_node의 왼쪽으로 내려간다.
    else                                              //key값이 find_node의 key보다 크다면
      find_node = find_node->right;                   //find_node느 find_node의 오른쪽으로 내려간다.
  }

  return NULL;                                        //while문이 종료되었을 때 find_node를 return 하지 못했다면 NULL을 반환한다. *key를 찾지 못함                            
}

node_t *rbtree_min(const rbtree *t)                   //rbtree에서 가장 작은 노드를 반환하는 함수
{
  node_t *min_node_left = t->root;                    //탐색이 끝난 후 min_node의 왼쪽 자식이 되는 노드
  node_t *min_node = t->nil;                          //min_node를 nil로 초기화
  
  while (min_node_left != t->nil) {                   //탐색 노드가 nil노드가 될 때까지 반복
    min_node = min_node_left;                         //min_node에 탐색 중인 노드를 대입
    min_node_left = min_node_left->left;              //탐색 노드를 왼쪽으로 이동
  }

  return min_node == t->nil ? NULL : min_node;        //min_node가 nil노드면 NULL, 아니면 min_node의 값을 반환
}

node_t *rbtree_max(const rbtree *t)                   //rbtree에서 가장 큰 노드를 반환하는 함수
{
  node_t *max_node_right = t->root;                   //탐색이 끝난 후 max_node의 오른쪽 자식이 되는 노드
  node_t *max_node = t->nil;                          //max_node를 nil로 초기화
  
  while (max_node_right != t->nil) {                  //탐색 노드가 nil노드가 될 때까지 반복
    max_node = max_node_right;                        //max_node에 탐색 중인 노드를 대입
    max_node_right = max_node_right->right;           //탐색 노드를 오른쪽으로 이동
  }

  return max_node == t->nil ? NULL : max_node;        //max_node가 nil노드면 NULL, 아니면 max_node의 값을 반환
}

int rbtree_erase(rbtree *t, node_t *p) {  
  
  // RB tree 내부의 ptr로 지정된 node를 삭제하고 메모리 반환
  // test 조건엔 없지만 삭제하는 노드 key 반환하도록 해뒀음 
  
  node_t *replace_node = t->nil;                      // erase_node의 자리를 대체할 노드
  node_t *erase_node = p;                             // 실제 삭제되는 노드 (처음엔 p 자신, 나중엔 successor가 될 수도 있음)
  color_t y_original_color = erase_node->color;

  
  if (p->left == t->nil) {                            // case 1: 왼쪽 자식이 없음 → 오른쪽 자식으로 대체 (혹은 nil)
    replace_node = p->right;
    _rb_transplant(t, p, p->right);
  }

  else if (p->right == t->nil) {                      // case 2: 오른쪽 자식이 없음 → 왼쪽 자식으로 대체
    replace_node = p->left;
    _rb_transplant(t, p, p->left);
  }
 
  else {                                              // case 3: 양쪽 자식 모두 있음
                                                      
    erase_node = _subtree_min(t, p->right);           // 후임자 erase_node = 오른쪽 서브트리의 최소 노드
    y_original_color = erase_node->color;
    replace_node = erase_node->right;

    if (erase_node != p->right) {                       // case 3-1: y가 p의 직속 오른쪽 자식이 아님      
      _rb_transplant(t, erase_node, erase_node->right); // erase_node 자리를 erase_node의 오른쪽 자식으로 대체
      
      erase_node->right = p->right;                     // erase_node에 p의 오른쪽 자식 연결
      erase_node->right->parent = erase_node;
    } else {                                            // case 3-2: erase_node가 p의 직속 오른쪽 자식 → transplant 생략했으므로 직접 부모 연결
      replace_node->parent = erase_node;
    }
    _rb_transplant(t, p, erase_node);                   // p 자리를 erase_node로 대체 (erase_node는 이제 p의 위치로 올라옴)

    erase_node->left = p->left;                // y에 p의 왼쪽 자식 연결
    erase_node->left->parent = erase_node;              // 삭제 전 p의 색을 y에게 전달 (트리 속성 유지용)
    
    erase_node->color = p->color;
  }
 
  if (y_original_color == RBTREE_BLACK) {               // 삭제된 노드가 BLACK일 경우 → 속성 위배 가능성 → fixup 필요
    _rb_delete_fixup(t, replace_node);
  }
  free(p);                                              // 삭제된 노드 메모리 해제
  p = NULL;

  return 0;
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


// int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
// //RB tree의 내용을 *key 순서대로* 주어진 array로 변환
// //array의 크기는 n으로 주어지며 tree의 크기가 n 보다 큰 경우에는 순서대로 n개 까지만 변환
// //array의 메모리 공간은 이 함수를 부르는 쪽에서 준비하고 그 크기는 n
//   int idx = 0;
//   _inorder_tree(t, t->root, arr, n, &idx);

//   return idx;
// }


/////////////////////////////////////////////////////////
//트리를 후위 순회하면서 노드의 메모리 해제
static void _delete_node(rbtree *t, node_t *deleit_node)
{ 
  if(deleit_node == t->nil)                             //현재 노드가 nil노드라면 
    return;                                             //return

  _delete_node(t, deleit_node->left);                   //왼쪽 자식을 free한다.
  _delete_node(t, deleit_node->right);                  //오른쪽 자식을 free한다.
  free(deleit_node);                                    //현재 자식을 free한다.
}

//이진트리의 조건을 지키면서 왼쪽으로 회전
static void _left_rotate(rbtree *t, node_t *standard_node)
{
  node_t *rotation_node = standard_node->right;         //rotation_node는 standard_node 노드의 오른쪽 자식
  standard_node->right = rotation_node->left;           //rotation_node의 왼쪽 서브트리를 standard_node의 오른쪽 서브트리로 회전한다.
  
  if(rotation_node->left != t->nil)                     //rotation_node의 왼쪽 서브트리가 비어 있지 않은 경우
    rotation_node->left->parent = standard_node;        //standard_node는 서브 트리의 부모가 된다.

  rotation_node->parent = standard_node->parent;        //standard_node의 부모가 rotation_node의 부모가 된다.

  if(standard_node->parent == t->nil)                   //standard_node의 부모가 nil노드라면, standard_node가 루트인 경우
    t->root = rotation_node;                            //트리의 root는 rotation_node가 된다.
  else if(standard_node == standard_node->parent->left) //standard_node가 부모의 왼쪽 자식이라면
    standard_node->parent->left = rotation_node;        //standard_node의 부모의 왼쪽 자식은 rotation_node가 된다.
  else                                                  //standard_node가 부모의 오른쪽 자식이라면
    standard_node->parent->right = rotation_node;       //standard_node의 부모의 오른쪽 자식은 rotation_node가 된다.

  rotation_node->left = standard_node;                  //standard_node는 rotation_node의 왼쪽 자식이 된다.
  standard_node->parent = rotation_node;                //standard_node의 부모는 rotation_node가 된다.
}

//이진트리의 조건을 지키면서 오른쪽으로 회전
static void _right_rotate(rbtree *t , node_t *standard_node)

{
  node_t *rotation_node= standard_node->left;           //rotation_node는 standard_node 노드의 왼쪽 자식
  standard_node->left = rotation_node->right;           //rotation_node의 오른쪽 서브트리를 standard_node의 왼쪽 서브트리로 회전한다.
  
  if(rotation_node->right != t->nil)                    //rotation_node의 오른쪽 서브트라가 비어 있지 않은 경우
    rotation_node->right->parent = standard_node;       //standard_node의 오른쪽 서브 트리의 부모가 된다.

  rotation_node->parent = standard_node->parent;        //standard_node의 부모가 rotation_node의 부모가 된다.
  
  if(standard_node->parent == t->nil)                   //standard_node의 부모가 nil노드라면, standard_node가 루트인 경우
    t->root = rotation_node;                            //트리의 root는 rotation_node가 된다.
  else if(standard_node == standard_node->parent->right)//standard_node가 부모의 오른쪽 자식이라면
    standard_node->parent->right = rotation_node;       //standard_node의 부모의 오른쪽 자식은 rotation_node가 된다.
  else                                                  //standard_node가 부모의 오른쪽 자식이라면
    standard_node->parent->left = rotation_node;        //standard_node의 부모의 왼쪽 자식은 rotation_node가 된다.

  rotation_node->right = standard_node;                 //standard_node는 rotation_node의 오른쪽 자식이 된다.
  standard_node->parent = rotation_node;                //standard_node의 부모는 rotation_node가 된다.
}

//삽입 시 조건 위반 생길 경우 수정하는 내부 함수 구현
static void _rb_insert_fixup(rbtree *t, node_t *standard_node)
{
  
  node_t *rotation_node = NULL;

  while (standard_node->parent->color == RBTREE_RED)                  // R(P)-R(N) 상황
  {
    if (standard_node->parent == standard_node->parent->parent->left) // 아부지가 할부지 왼팔이면
    {
      // 삼촌은 할부지 오른팔
      rotation_node = standard_node->parent->parent->right;

      // 삼촌도 R이면 -> B(G), R(P, U, N)인 상태이므로 case 1
      if (rotation_node->color == RBTREE_RED)
      { 
        standard_node->parent->color = RBTREE_BLACK;
        rotation_node->color = RBTREE_BLACK;
        standard_node->parent->parent->color = RBTREE_RED;
        standard_node = standard_node->parent->parent; // 할부지부터 다시 검사 -> while 걸릴 수도, 안 걸릴 수도  
      }

      // 삼촌이 B이면, 허리 핀 경우와 허리 굽은 경우 판단
      else
      {
        if (standard_node == standard_node->parent->right) // 허리 굽은 경우(<, LR) case 2
        {
          standard_node = standard_node->parent;
          _left_rotate(t, standard_node);
        }

        // 허리 핀 경우(/, LL) case 1 (case 2를 해결하고 온 후에도 걸림)
        standard_node->parent->color = RBTREE_BLACK;
        standard_node->parent->parent->color = RBTREE_RED;
        _right_rotate(t, standard_node->parent->parent);
      }
    } 
    
    else // 아부지가 할부지 오른팔이면
    {
      // 삼촌은 왼팔
      rotation_node = standard_node->parent->parent->left;

      // 삼촌도 R이면 -> B(G), R(P, U, N)인 상태이므로 case 1
      if (rotation_node->color == RBTREE_RED)
      { 
        standard_node->parent->color = RBTREE_BLACK;
        rotation_node->color = RBTREE_BLACK;
        standard_node->parent->parent->color = RBTREE_RED;
        standard_node = standard_node->parent->parent; // 할부지부터 다시 검사 -> while 걸릴 수도, 안 걸릴 수도  
      }

      // 삼촌이 B이면, 허리 핀 경우와 허리 굽은 경우 판단
      else
      {
        if (standard_node == standard_node->parent->left) // 허리 굽은 경우(>, RL) case 2
        {
          standard_node = standard_node->parent;
          _right_rotate(t, standard_node);
        }

        // 허리 핀 경우(\, RR) case 1 (case 2를 해결하고 온 후에도 걸림)
        standard_node->parent->color = RBTREE_BLACK;
        standard_node->parent->parent->color = RBTREE_RED;
        _left_rotate(t, standard_node->parent->parent);
      }
    }
  }  
  // 2번 조건(루트는 블랙) 위배 시 그냥 블랙으로 바꿔줘서 처리하면 끝  
  t->root->color = RBTREE_BLACK; 
}

// 지우려고 하는 노드의 successor를 찾는 내부 함수 구현
static node_t *_subtree_min(const rbtree *t, node_t *node) {
  while (node->left != t->nil)
    node = node->left;
  return node;
}

//switch_node 와 node의 부모를 바꾸는 함수
static void _rb_transplant(rbtree *t, node_t *switch_node, node_t *node) {
  if (switch_node->parent == t->nil) 
    t->root = node;
  else if (switch_node == switch_node->parent->left) 
    switch_node->parent->left = node;
  else 
    switch_node->parent->right = node;

  node->parent = switch_node->parent;
}

// 삭제 시 조건 위반 생길 경우 수정하는 내부 함수 구현
static void _rb_delete_fixup(rbtree *t, node_t *node) {
  
  node_t *brother_node = t->nil;
  
  while ((node != t->root) && (node->color == RBTREE_BLACK)) 
  {
    if (node == node->parent->left) // 나(DB) 왼쪽, 형제는 오른쪽
    {
      brother_node = node->parent->right;
      
      if (brother_node->color == RBTREE_RED) // case1 (형제는 red)
        {
          brother_node->color = RBTREE_BLACK;
          node->parent->color = RBTREE_RED;
          _left_rotate(t, node->parent);
          brother_node = node->parent->right; // rotate 후, x의 형제가 바뀌었으니 w로 가리키는 곳 업데이트
        }
      
      if ((brother_node->left->color == RBTREE_BLACK)&&(brother_node->right->color == RBTREE_BLACK)) // case2 (형제, 형제 자식 블랙)
        {  
          brother_node->color = RBTREE_RED;
          node = node->parent;
        }
      
      else // 형제는 black, 형제의 자식 중 red가 있을 경우
      {
        if (brother_node->right->color == RBTREE_BLACK) // case3 (형제의 왼쪽 자식이 red, 나는 현재 왼쪽 자식이므로 더 가까운 조카가 빨강)
        {
          brother_node->left->color = RBTREE_BLACK;
          brother_node->color = RBTREE_RED;
          _right_rotate(t, brother_node);
          brother_node = node->parent->right;
        }
      
        // case4 (형제의 오른쪽 자식이 red, 나와 먼 조카)
        brother_node->color = node->parent->color;
        node->parent->color = RBTREE_BLACK;
        brother_node->right->color = RBTREE_BLACK;
        _left_rotate(t, node->parent);
        node = t->root; // case4의 해결로 모든 조건이 충족됐으므로 while 문 탈출을 위한 조건을 걸어줌
      }
    }

    else // 나(DB) 오른쪽, 형제는 왼쪽
    {
      brother_node = node->parent->left;
      
      if (brother_node->color == RBTREE_RED) // case1 (형제는 red)
        {
          brother_node->color = RBTREE_BLACK;
          node->parent->color = RBTREE_RED;
          _right_rotate(t, node->parent);
          brother_node = node->parent->left; // rotate 후, x의 형제가 바뀌었으니 w로 가리키는 곳 업데이트
        }
      
      if ((brother_node->right->color == RBTREE_BLACK)&&(brother_node->left->color == RBTREE_BLACK)) // case2 (형제, 형제 자식 블랙)
        {  
          brother_node->color = RBTREE_RED;
          node = node->parent;
        }
      
      else // 형제는 black, 형제의 자식 중 red가 있을 경우
      {
      
        if (brother_node->left->color == RBTREE_BLACK) // case3 (형제의 오른쪽 자식이 red, 나는 현재 오른쪽 자식이므로 더 가까운 조카가 빨강)
        {
          brother_node->right->color = RBTREE_BLACK;
          brother_node->color = RBTREE_RED;
          _left_rotate(t, brother_node);
          brother_node = node->parent->left;
        }
      
        // case4 (형제의 왼쪽 자식이 red, 나와 먼 조카)
        brother_node->color = node->parent->color;
        node->parent->color = RBTREE_BLACK;
        brother_node->left->color = RBTREE_BLACK;
        _right_rotate(t, node->parent);
        node = t->root; // case4의 해결로 모든 조건이 충족됐으므로 while 문 탈출을 위한 조건을 걸어줌
      }
    }

  }
  node->color = RBTREE_BLACK; // 계속 위로 올라가서 x가 루트가 됐을 경우, 루트 블랙 조건 보장
}

// 중위순회(n만큼) 내부 함수 구현
static void _inorder_tree(const rbtree *t, node_t *node, key_t *arr, size_t n, int *idx) {
  
  if (node == t->nil || *idx >= n) return;

  _inorder_tree(t, node->left, arr, n, idx); 
  if (*idx < n) arr[(*idx)++] = node->key; 
  _inorder_tree(t, node->right, arr, n, idx);

}

// static int _inorder_tree(const rbtree *t, node_t *node, key_t* arr, size_t n, size_t *idx)
// {
//   if(node == t->nil || *idx >= n)
//     return 0;

//     _inorder_tree(t, node->left, arr, n, &idx);
//     if(*idx < n) 
//       arr[(*idx)++] = node->key;
//     _inorder_tree(t, node->right, arr, n, &idx);
// }