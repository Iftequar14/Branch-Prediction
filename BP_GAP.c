#include <math.h>
#include <stdio.h>  
#include <stdlib.h>   
#include <string.h>
#include <ctype.h>

int hexToDec(char hex[])
{
    int base = 1;
    int decimalValue = 0;
    int i;
    for (i = strlen(hex) - 1; i >= 0; i--) 
    {
        hex[i] = toupper(hex[i]);
        if (hex[i] >= '0' && hex[i] <= '9')
        {
            decimalValue += (hex[i] - 48) * base;
            base = base * 16;
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F') 
        {
            decimalValue += (hex[i] - 55) * base;
            base = base * 16;
        }
    }
    return decimalValue;
}

int binaryToDecimal(int binary[])
{
    int decimalValue = 0;
    int base = 1;
    int i;
    for (i = 7; i >= 0; i--) 
    {
        if (binary[i] == 1)
        {
            decimalValue += base;
        }
        base = base * 2;
    }
    return decimalValue;
}

int main(int argc, char *argv[]) 
{
   int numBits = 12, bhrSize = 8, i, j;
   int numEntries = pow(2, numBits);
   int bhrCount = pow(2, bhrSize);
   int pht[numEntries][bhrCount];
   int bhr[bhrSize];
   int hexAddr;
   int prediction, correctPredictions = 0, totalPredictions = 0;
   int bhrValue;
   
   unsigned lastNBits;
   char branchAddress[8], outcome, binOutcome;
   unsigned mask;
   float accuracy = 0.0, hardwareCost;
   FILE *inputFile, *outputFile;
   
   inputFile = fopen(argv[1], "r");
   outputFile = fopen("Output_GAP.txt", "w");
   
   if (inputFile == NULL || outputFile == NULL)  
   {
     printf("Error! One or more files cannot be opened");
     exit(1);
   }
   
   // Initialize bhr to 11...1
   for (i = 0; i < bhrSize; i++)
   {
      bhr[i] = 0;
   }
   
   // Initialize pht to 11
   for (i = 0; i < numEntries; i++)
   {
      for (j = 0; j < bhrCount; j++)
      {
         pht[i][j] = 10;
      }
   }

    fprintf(outputFile, "address\toutcome\tprediction\tcorrect/miss\n"); 
   
       
   while (fscanf(inputFile, "%s %c %c", branchAddress, &outcome, &binOutcome) != EOF) 
   {
        int flag = 0;
        hexAddr = hexToDec(branchAddress); // Convert hex branch address to decimal
        mask = (1 << numBits) - 1;
        lastNBits = hexAddr & mask;    // Extract the decimal value of last n bits of the address
        
        bhrValue = binaryToDecimal(bhr);        // Convert bhr value to decimal
        
        // Prediction
        prediction = (pht[lastNBits][bhrValue] == 11 || pht[lastNBits][bhrValue] == 10) ? 1 : 0;

        
        // Update prediction statistics
        if ((outcome == '+' && prediction) || (outcome == '-' && !prediction))
        {
          correctPredictions += 1;
          flag = 1;
        }
        totalPredictions += 1;
        fprintf(outputFile, "%s\t%c\t%d\t%d\n", branchAddress,outcome,prediction,flag); 
       
        // Update pht
        if (outcome == '+')
        {
            if (pht[lastNBits][bhrValue] == 10){
                pht[lastNBits][bhrValue] = 11;
            }
            else if (pht[lastNBits][bhrValue] == 01){
                pht[lastNBits][bhrValue] = 10;
            }
            else if (pht[lastNBits][bhrValue] == 00){
                pht[lastNBits][bhrValue] = 01;
            }
        }
        else 
        {
            if (pht[lastNBits][bhrValue] == 11){
                pht[lastNBits][bhrValue] = 10;
            }
            else if (pht[lastNBits][bhrValue] == 10){
                pht[lastNBits][bhrValue] = 01;
            }
            else if (pht[lastNBits][bhrValue] == 01){
                pht[lastNBits][bhrValue] = 00;
            }
        }
        
        // Update bhr      
        for (i = 0; i < bhrSize - 1; i++)
        {              
            bhr[i] = bhr[i + 1];
        }
        bhr[bhrSize - 1] = (outcome == '+') ? 1 : 0; 
	}
   
   // Calculate accuracy and hardware cost
   accuracy = (float)(correctPredictions * 100) / totalPredictions;
   hardwareCost = (float)(bhrSize + numEntries * bhrCount * 2) / 8192;
   
   // Display results
   printf("Total Branches: %d\n", totalPredictions);
   printf("Branches predicted correctly: %d\n", correctPredictions);
   printf("Branch prediction accuracy is %f\n", accuracy);
   printf("The hardware cost associated with Branch Prediction using GAp: %f KB\n", hardwareCost);
   
   // Close files
   fclose(inputFile);
   fclose(outputFile);
 
   return 0; 
}
