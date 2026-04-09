#include <stdio.h>

int main(){
 int a, b;
 char op;
 printf("Digite: ");
 scanf("%d %c %d", &a, &op, &b);
 if (op == '=') {
  printf("resultado: %d\n.", a + b);
 }
}