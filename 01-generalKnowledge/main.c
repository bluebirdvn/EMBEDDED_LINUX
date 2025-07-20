#include <stdio.h>
#include "display.h"
#include "calculator.h"

int main() {
	int a, b;
	a = 1000;
	b = 2000;
	int s1 = cal_sum(a, b);
	int s2 = cal_sub(a, b);
	display(s1, s2);
	return 0;
}

