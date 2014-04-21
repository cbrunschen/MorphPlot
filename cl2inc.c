//
//  cl2inc.c
//  MorphPlot
//
//  Created by Christian Brunschen on 15/04/2014.
//
//

#include <stdio.h>
#include <string.h>

// Converts the input into a C-string-escaped form, suitable for inclusion into C(++) source code.
int main(int argc, char **argv) {
  // the following two arrays must be maintained in matching order:
  static char *inputs = "\a\b\f\r\t\v\\\"\'";
  static char *outputs = "abfrtv\\\"\'";

  int c;
  char *pos;
  printf("\"\\\n");
  while ((c = getc(stdin)) != EOF) {
    if (c == '\n') {
      printf("\\n\\\n");
    } else if ((pos = strchr(inputs, c))) {
      printf("\\%c", outputs[pos-inputs]);
    } else {
      printf("%c", c);
    }
  }
  printf("\"\n");
}