/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("&argc: %p argv: %p env: %p\n", &argc, argv, env);
  printf("&a: %p &b: %p &c: %p\n", &a, &b, &c);

//(1). Write C code to print values of argc and argv[] entries
//------------------------------------------------------
  printf("argc: %d\n", argc);

  printf("argv: ");
  for(int i = 0; i < argc; i++) {
      printf("%s, ", argv[i]);
  }
  printf("\n");
  //----------------------------------------------------

  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
  return 0;
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  // write C code to PRINT ADDRESS OF d, e, f
  //-------------------------------------------
    printf("&d: %p, &e: %p, &f: %p\n", &d, &e, &f);
  //-------------------------------------------
  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
  return 0;
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");
  // write C code to PRINT ADDRESS OF g,h,i
  //-------------------------------------------
    printf("&g: %p, &h: %p, &i: %p\n", &g, &h, &i);
  //-------------------------------------------
  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
  return 0;
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");
  // write C cdoe to PRINT ADDRESS OF u,v,w,i,p;
  //-------------------------------------------
    printf("&u: %p, &v: %p, &w: %p, &i: %p, &p: %p\n", &u, &v, &w, &i, &p);
  //-------------------------------------------
  u=10; v=11; w=12; i=13;

  FP = (int *)getebp();  // FP = stack frame pointer of the C() function
//  print FP value in HEX  

//---------------------------------------------------
printf("Current FP Val: %p\n", FP);
//---------------------------------------------------

//(2). Write C code to print the stack frame link list.
//-----------------------------------------------------
int *j = FP;

printf("----- FP list -----\n");
while(j != 0) {
    printf("%p -> ", j);
    j = *j;
}
printf("%x\n", j);
//-----------------------------------------------------

 p = (int *)&u;

 //(3). Print the stack contents from p to the frame of main()
//     YOU MAY JUST PRINT 128 entries of the stack contents.

//-------------------------------------------------
  int q = 0;

  printf("----- Stack Contents -----\n");
  while(q < 128) {
    printf("%3d: p: %p -> %x\n", q, p, *p);
    p++;
    q++;
  }
//-------------------------------------------------

//(4). On a hard copy of the print out, identify the stack contents
//    as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.
printf("exit C\n");
return 0;
}
