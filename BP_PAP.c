#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hexToDec(char hex[]) {
  int base = 1;
  int decimalValue = 0;
  int i;
  for (i = strlen(hex) - 1; i >= 0; i--) {
    hex[i] = toupper(hex[i]);
    if (hex[i] >= '0' && hex[i] <= '9') {
      decimalValue += (hex[i] - 48) * base;
      base = base * 16;
    } else if (hex[i] >= 'A' && hex[i] <= 'F') {
      decimalValue += (hex[i] - 55) * base;
      base = base * 16;
    }
  }
  return decimalValue;
}

int binaryToDecimal(int binary[]) {
  int decimalValue = 0;
  int base = 1;
  int i;
  for (i = 7; i >= 0; i--) {
    if (binary[i] == 1) {
      decimalValue += base;
    }
    base = base * 2;
  }
  return decimalValue;
}

int main(int argc, char *argv[]) {
  int numBits = 12, bhrSize = 8, i, j;
  int numEntries = pow(2, numBits);
  int bhrCount = pow(2, bhrSize);
  int pht[bhrCount][numEntries];
  int bhr[numEntries][bhrSize];
  int temp[bhrSize];
  int hexAddr;
  int prediction, correctPredictions = 0, totalPredictions = 0;
  int bhrValue;

  unsigned int lastNBits;
  char branchAddress[8], outcome, binOutcome;
  unsigned int mask;
  float accuracy = 0.0, hardwareCost;
  FILE *inputFile, *outputFile;

  // Open files for reading and writing
  inputFile = fopen(argv[1], "r");
  outputFile = fopen("Output_PAP.txt", "w");

  if (inputFile == NULL || outputFile == NULL) {
    printf("Error! One or more files cannot be opened");
    exit(1);
  }

  // Initialize bhr to 11...1
  for (i = 0; i < numEntries; i++) {
    for (j = 0; j < bhrSize; j++) {
      bhr[i][j] = 0;
    }
  }

  // Initialize everything in pht to 11
  for (i = 0; i < bhrCount; i++) {
    for (j = 0; j < numEntries; j++) {
      pht[i][j] = 10;
    }
  }

  fprintf(outputFile, "address\toutcome\tprediction\tcorrect/miss\n");

  // Process input file
  while (fscanf(inputFile, "%s %c %c", branchAddress, &outcome, &binOutcome) !=
         EOF) {
    int flag = 0;
    hexAddr = hexToDec(branchAddress); // Convert hex branch address to decimal
    mask = (1 << numBits) - 1;
    lastNBits = hexAddr &
                mask; // Extract the decimal value of last n bits of the address

    memcpy(temp, bhr[lastNBits], sizeof(bhr[lastNBits]));
    bhrValue = binaryToDecimal(temp); // Bhr value in decimal

    // Prediction
    prediction =
        (pht[bhrValue][lastNBits] == 11 || pht[bhrValue][lastNBits] == 10) ? 1
                                                                           : 0;

    // Update prediction statistics
    if ((outcome == '+' && prediction) || (outcome == '-' && !prediction)) {
      correctPredictions += 1;
      flag = 1;
    }

    totalPredictions += 1;
    fprintf(outputFile, "%s\t%c\t%d\t%d\n", branchAddress, outcome, prediction,
            flag);

    // Update pht
    if (outcome == '+') {
      if (pht[bhrValue][lastNBits] == 10) {
        pht[bhrValue][lastNBits] = 11;
      } else if (pht[bhrValue][lastNBits] == 01) {
        pht[bhrValue][lastNBits] = 10;
      } else if (pht[bhrValue][lastNBits] == 00) {
        pht[bhrValue][lastNBits] = 01;
      }
    } else {
      if (pht[bhrValue][lastNBits] == 11) {
        pht[bhrValue][lastNBits] = 10;
      } else if (pht[bhrValue][lastNBits] == 10) {
        pht[bhrValue][lastNBits] = 01;
      } else if (pht[bhrValue][lastNBits] == 01) {
        pht[bhrValue][lastNBits] = 00;
      }
    }

    // Update bhr
    for (j = 0; j < bhrSize - 1; j++) {
      bhr[lastNBits][j] = bhr[lastNBits][j + 1];
    }
    bhr[lastNBits][bhrSize - 1] = (outcome == '+') ? 1 : 0;
  }

  // Calculate accuracy and hardware cost
  accuracy = (float)(correctPredictions * 100) / totalPredictions;
  hardwareCost =
      (float)(numEntries * bhrSize + numEntries * bhrCount * 2) / 8192;

  // Display results
  printf("Total Branches: %d\n", totalPredictions);
  printf("Branches predicted correctly: %d\n", correctPredictions);
  printf("Branch prediction accuracy is %f\n", accuracy);
  printf(
      "The hardware cost associated with Branch Prediction using GAp: %f KB\n",
      hardwareCost);

  // Close files
  fclose(inputFile);
  fclose(outputFile);

  return 0;
}
