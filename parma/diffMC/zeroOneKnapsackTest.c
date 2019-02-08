#include "zeroOneKnapsack.h"
#include <stdio.h>
#include <pcu_util.h>
#include <stdlib.h>

void test1() {
  size_t maxw = 4;
  size_t n = 4;
  size_t w[4] = {1,2,3,4};
  size_t v[4] = {0,1,4,2};
  size_t val, size;
  size_t* soln;
  Knapsack k = makeKnapsack(maxw, n, w, v);
  val = solve(k);
  printTable(k);
  lion_oprint(1,"val %lu\n", val);
  PCU_ALWAYS_ASSERT(val == 4);
  soln = getSolution(k, &size);
  lion_oprint(1,"size %lu soln[0] %lu\n", size, soln[0]);
  PCU_ALWAYS_ASSERT(size == 1);
  PCU_ALWAYS_ASSERT(soln[0] == 2);
  destroyKnapsack(k);
  free(soln);
}

void test2() {
  /* http://cse.unl.edu/~goddard/Courses/CSCE310J/Lectures/Lecture8-DynamicProgramming.pdf */
  size_t maxw = 5;
  size_t n = 4;
  size_t w[4] = {2,3,4,5};
  size_t v[4] = {3,4,5,6};
  size_t val, size;
  size_t* soln;
  Knapsack k = makeKnapsack(maxw, n, w, v);
  val = solve(k);
  printTable(k);
  lion_oprint(1,"val %lu\n", val);
  PCU_ALWAYS_ASSERT(val == 7);
  soln = getSolution(k, &size);
  lion_oprint(1,"size %lu soln[0] %lu soln[1] %lu\n", size, soln[0], soln[1]);
  PCU_ALWAYS_ASSERT(size == 2);
  PCU_ALWAYS_ASSERT(soln[0] == 1 && soln[1] == 0);
  destroyKnapsack(k);
  free(soln);
}

int main() {
  test1();
  test2();
  return 0;
}
